//
//  power.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "power_msr.h"

Power_MSR::Power_MSR() {

}

Power_MSR::~Power_MSR() {

}

bool Power_MSR::open_msr(int core) {

	char msr_filename[PATH_LENGTH];

	sprintf(msr_filename, MSR_PATH, core);
	fd[core] = open(msr_filename, O_RDONLY);
	if (fd[core] < 0) {

		if ( errno == ENXIO ) {
			printf("rdmsr: No CPU %d\n", core);
			return false;
		} else if ( errno == EIO ) {
			printf("rdmsr: CPU %d doesn't support MSRs\n", core);
			return false;
		} else {
			printf("Trying to open %s\n", msr_filename);
			return false;
		}

	}

	return true;
}

long long Power_MSR::read_msr(int core, int which) {

	uint64_t data;

	if (pread(fd[core], &data, sizeof(data), which) != sizeof(data)) {
		printf("rdmsr:pread error\n");
		return 0;
	}

	return (long long)data;
}

void Power_MSR::close_msr(int core) {

	close(fd[core]);
}


bool Power_MSR::read_before() {

	pp0_before = 0.0;

	for (int i = 0; i < core_count; i++) {

		open_msr(i);

		long long result = read_msr(i, MSR_RAPL_POWER_UNIT);
		cpu_energy_units[i] = pow(0.5, (double)((result >> 8) & 0x1f));
#if 0
		/* This read is for processor die, not necessary now*/
		result = read_msr(i, MSR_PKG_ENERGY_STATUS);
		package_before = (double) result * cpu_energy_units;
#endif
		result = read_msr(i, MSR_PP0_ENERGY_STATUS);
		pp0_before += (double) result * cpu_energy_units[i];

#if 0
		/* This read is for on core GPU, not necessary now*/
		result = read_msr(i, MSR_PP1_ENERGY_STATUS);
		pp1_before=(double)result*cpu_energy_units;
#endif

	}

	return true;

}

double Power_MSR::read_after() {

	pp0_after = 0;

	for (int i = 0; i < core_count; i++) {

		long long result = read_msr(i, MSR_PP0_ENERGY_STATUS);
		pp0_after += (double) result * cpu_energy_units[i];

		close_msr(i);

	}

	return pp0_after - pp0_before;

}
