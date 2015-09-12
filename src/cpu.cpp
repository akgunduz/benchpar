//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//

#include "cpu.h"

CPU::CPU() {

	type = PROC_UNKNOWN;
	coreCount = 0;
	powerCoreCount = 0;
	if (detectCPU()) {
		powerCoreCount = cores[0].type == CORE_HASWELL ||
				cores[0].type == CORE_HASWELL_EP ||
				cores[0].type == CORE_BROADWELL ? coreCount : 1;
	}

}

bool CPU::detectCPU() {

#ifndef __linux__
	printf("CPU can not detected in non-linux platforms\n");
	return true;
#endif

	FILE *fd = fopen(CPUINFO_PATH,"r");
	if (fd == NULL) {
		return false;
	}

	char buffer[PATH_LENGTH];
	char modelname[PATH_LENGTH];
	uint32_t part;

	while(1) {

		char *result = fgets(buffer, PATH_LENGTH, fd);
		if (result == NULL) {
			break;
                        
		} else if (!strncmp(result, "processor", 9)) {
			coreCount++;
                        
		} else if (!strncmp(result, "vendor_id", 9)) {
                        sscanf(result,"%*s%*s%s", buffer);
			if (!strncmp(buffer, "GenuineIntel", 12)) {
                                strcpy(modelname, "Intel");
				type = PROC_INTEL;
			}
                        
                } else if (!strncmp(result, "model name", 10)) {
			sscanf(result,"%*s%*s%*s%s", buffer);
			if (!strncmp(buffer, "ARMv7", 5)) {
                                strcpy(modelname, "ARMv7");
				type = PROC_ARM;
				continue;
			}

		} else if (type == PROC_INTEL && !strncmp(result, "model", 5)) {
			sscanf(result,"%*s%*s%d", &part);
			switch(part) {
				case CPU_SANDYBRIDGE:
					cores[coreCount - 1].type = CORE_SANDYBRIDGE;
					break;
				case CPU_SANDYBRIDGE_EP:
					cores[coreCount - 1].type = CORE_SANDYBRIDGE_EP;
					break;
				case CPU_IVYBRIDGE:
					cores[coreCount - 1].type = CORE_IVYBRIDGE;
					break;
				case CPU_IVYBRIDGE_EP:
					cores[coreCount - 1].type = CORE_IVYBRIDGE_EP;
					break;
				case CPU_HASWELL:
					cores[coreCount - 1].type = CORE_HASWELL;
					break;
				case CPU_HASWELL_EP:
					cores[coreCount - 1].type = CORE_HASWELL_EP;
					break;
				case CPU_BROADWELL:
					cores[coreCount - 1].type = CORE_BROADWELL;
					break;
				default:
					cores[coreCount - 1].type = CORE_UNKNOWN;
					break;
			}

		} else if (type == PROC_ARM && !strncmp(result, "CPU part", 8)) {
			sscanf(result,"%*s%*s%*s%x", &part);
			cores[coreCount - 1].type = part == 0xC0F ? CORE_A15 : CORE_A7;
		}

	}

	fclose(fd);

	printf("Processor %s found with %d cores : ", modelname, coreCount);

	if (coreCount == 0) {
		printf("\n");
		return false;
	}

	int count = 1;
	CORETYPE ref = cores[0].type;

	for (int i = 1; i < coreCount; i++) {
		if (ref != cores[i].type) {
			printf("%d x %s, ", count, names[ref]);
			ref = cores[i].type;
			count = 0;
		}
		count++;
	}

	printf("%d x %s\n", count, names[ref]);

	return true;
}

