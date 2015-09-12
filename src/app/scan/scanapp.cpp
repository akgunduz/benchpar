//
//  scanapp.cpp
//  ScanCalculator
//
//  Created by Haluk AKGUNDUZ on 03/09/15.
//  Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//

#include "scanapp.h"

ScanApp::ScanApp(CPU *c, GPU *g, Power *pw) :
		App(c, g, pw) {

	loadGPUKernel();
}

ScanApp::~ScanApp() {

	unLoadGPUKernel();
}

int ScanApp::getFuncModeCount(FUNCTYPE functype) {

	switch(functype) {
		case FUNCTYPE_CPU:
			return SCANTYPE_GPU_STD;
		case FUNCTYPE_GPU:
			return SCANTYPE_MAX - SCANTYPE_GPU_STD;
		case FUNCTYPE_ALL:
			return SCANTYPE_MAX;
	}

	return 0;
}

bool ScanApp::loadGPUKernel() {

	if (!gpu->getEnabled()) {
		return false;
	}

	char file[PATH_LENGTH];

	sprintf(file, "%s/scan.cl", getcwd(NULL, 0));

	bool res = gpu->createBuildProgramFromFile(0, NULL, file);

	if (!res) {
		printf("Could not load GPU kernel\n");
		return false;
	}

	res = gpu->createCommandQueue(0, 0);
	if (!res) {
		printf("Could not create GPU Command Queue\n");
		clReleaseProgram(gpu->clProgram);
		return false;
	}

	gpu->localWorkSize[0] = 16;
	gpu->localWorkSize[1] = 16;

	gpu_kernel_loaded = true;

	return true;
}

void ScanApp::unLoadGPUKernel() {

	if (gpu_kernel_loaded) {
		clReleaseProgram(gpu->clProgram);
		clReleaseCommandQueue(gpu->clCommandQue);
	}
}

Scan* ScanApp::calculate(Scan *A, int modeID, bool print, int repeat) {

	if (!A->check()) {
		return NULL;
	}

	Scan* calculated = new Scan(A->getSize());

	Timer t;
	double t_min = 10000000.0f, t_max = 0.0f, t_total = 0.0f, consumed = 0.0f;

	if (power != NULL) {
		power->read_before();
	}

	for (int i = 0; i < repeat; i++) {

		t.snapshot();

		if (!(A->*(A->scanners[modeID].f))(calculated, gpu)) {
			printf("\nScan Method: %s failed\n", A->scanners[modeID].id);
			delete calculated;
			return NULL;
		}
		double t_diff = t.getdiff();

		if (t_min > t_diff) {
			t_min = t_diff;
		}
		if (t_diff > t_max) {
			t_max = t_diff;
		}

		t_total += t_diff;
	}

	if (power != NULL) {
		consumed = power->read_after();
	}

	if (print) {
		calculated->printOut();
	}

	printf("\nScan Method: %s \n", A->scanners[modeID].id);
	printf("Scan Time: %.3lf seconds!!!\n", t_total);
	printf("Min Time: %.3lfs, Max Time: %.3lfs, Avg Time: %.3lfs\n", t_min, t_max, t_total / repeat);

	if (power != NULL && power->getMode() != POWER_OFF) {
		printf("Power -> Method: %s, Consumed: %.3lf Joules!!!\n",
				power->getName(), consumed);
	}

	return calculated;
}

bool ScanApp::process(char fileInputs[][255]) {

	Timer t;

	Scan *A;

	t.snapshot();

	printf("\n=========================================\n");
	printf("Loading Scan Input Files from Storage... \n");

	try {

		A = new Scan(fileInputs[0]);

	} catch(const std::runtime_error e) {

		printf("Scan A could not created!!!, Exception : %s\n", e.what());
		return false;
	}

	printf("Calculate Started... Size : %dK\n", A->getSize() / 1024);

	if (seqID == SEQTYPE_NONE) {

		Scan *C = calculate(A, modeID, print_enabled, repeat);
		if (C != NULL && sanityID != INVALID_SANITY) {
			Scan *D = calculate(A, sanityID, false, 1);

			if (D != NULL) {
				if (C->compare(D)) {
					printf("Sanity check passed\n");
				} else {
					printf("Sanity check failed\n");
				}
				delete D;
			}

			delete C;
		}


	} else {

		int startIndex = 0;
		int count = 0;
		switch(seqID) {
			case SEQTYPE_ALL:
			default:
				startIndex = SCANTYPE_CPU_STD;
				count = SCANTYPE_MAX - SCANTYPE_CPU_STD;
				break;
			case SEQTYPE_CPU:
				startIndex = SCANTYPE_CPU_STD;
				count = SCANTYPE_GPU_STD - SCANTYPE_CPU_STD;
				break;
			case SEQTYPE_GPU:
				startIndex = SCANTYPE_GPU_STD;
				count = SCANTYPE_MAX - SCANTYPE_GPU_STD;
				break;
		}

		for (int i = startIndex; i < startIndex + count; i++) {
			calculate(A, i, false, 1);
		}
	}

	double t_diff = t.getdiff();
	printf("\nTotal Time: %.3lf seconds!!!\n\n", t_diff);

	delete A;

	return true;
}

bool ScanApp::processDir(const char path[255]) {

	char fileInputs[1][255];

	struct dirent *ent;

	DIR *dir = opendir(path);
	if (dir == nullptr) {
		printf ("Directory : %s could not opened\n err: %d", path, errno);
		return false;
	}

	while((ent = readdir(dir)) != nullptr) {

		if (ent->d_type != DT_REG) {
			continue;
		}

		if (strncmp(ent->d_name, "ScanInput", 9) != 0) {
			continue;
		}

		sprintf(fileInputs[0], "%s/%s", path, ent->d_name);
		bool status = process(fileInputs);
		if (!status) {
			return false;
		}

	}

	return true;
}

bool ScanApp::run(int argc, const char argv[][ARGV_LENGTH]) {

	char fileInputs[2][255];
	int fileIndex = 0;
	bool dirMode = true;

	for (int i = 0; i < argc; i++) {

		if (!strcmp (argv[i], "-c")) {

			if (i + 2 >= argc) {
				printf("Scan Creator needs printID, size values \n");
				return 0;
			}

			return creator((unsigned)atoi(argv[i+1]), (unsigned)atoi(argv[i+2]), 0);

		} else {

			dirMode = false;

			if (fileIndex == 1) {
				continue;
			}

			if (isdigit(argv[i][0])) {
				sprintf(fileInputs[fileIndex++], "scan/ScanInput_%s", argv[i]);
			} else {
				sprintf(fileInputs[fileIndex++], "scan/%s", argv[i]);
			}

		}

	}

	if (dirMode) {

		printf("Test is running in Directory Mode with %d repeats\n", repeat);

		return processDir("scan");

	} else {

		if (fileIndex < 1) {
			printf("Scan File Inputs did not entered\n");
			return false;
		}

		printf("Test is running with %d repeats\n", repeat);

		return process(fileInputs);
	}
}


bool ScanApp::creator(uint32_t printID, uint32_t size, uint32_t unused) {

	Scan *A = new Scan(size, true);
	A->printToFile(printID);
	delete A;
	return true;
}
