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

bool ScanApp::loadGPUKernel() {

	if (!gpu->getEnabled()) {
		return false;
	}

	char file[PATH_LENGTH];

	sprintf(file, "%s/matMul.cl", getcwd(NULL, 0));

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

	printf("Loading Scan Input Files from Storage... \n");

	try {

		A = new Scan(fileInputs[0]);

	} catch(const std::runtime_error e) {

		printf("Scan A could not created!!!, Exception : %s\n", e.what());
		return false;
	}

	printf("Calculate Started... \n");

	if (seqID == SEQTYPE_NONE) {

		Scan *C = calculate(A, modeID, print_enabled, repeat);
		if (C != NULL && sanityID < SCANTYPE_MAX) {
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
	printf("\nTotal Time: %.3lf seconds!!!\n", t_diff);

	delete A;

	return true;
}

bool ScanApp::run(int argc, const char argv[][ARGV_LENGTH]) {

	char fileInputs[2][255];
	int fileIndex = 0;

	for (int i = 0; i < argc; i++) {

		if (!strcmp (argv[i], "-c")) {

			if (i + 2 >= argc) {
				printf("Scan Creator needs printID, size values \n");
				return 0;
			}

			Scan *A = new Scan((unsigned)atoi(argv[i+2]), true);
			A->printToFile((unsigned)atoi(argv[i+1]));
			delete A;
			return true;

		} else if (!strcmp (argv[i], "-m")) {

			if (i + 1 >= argc) {
				printf("Mode Setting needs modeID \n");
				return 0;
			}

			if (isdigit(argv[++i][0])) {
				modeID = (SCANTYPE) atoi(argv[i]);
				if (modeID >= SCANTYPE_MAX) {
					modeID = SCANTYPE_CPU_STD;
				}
			} else {
				switch(argv[i][0]) {
					case 'a':
					default:
						seqID = SEQTYPE_ALL;
						break;
					case 'c':
						seqID = SEQTYPE_CPU;
						break;
					case 'g':
						seqID = SEQTYPE_GPU;
						break;
				}
			}

		} else if (!strcmp (argv[i], "-s")) {

			if (i + 1 >= argc) {
				printf("Sanity Setting needs modeID \n");
				return 0;
			}

			if (isdigit(argv[++i][0])) {
				sanityID = (SCANTYPE) atoi(argv[i]);
			}

		} else {

			if (fileIndex == 2) {
				printf("Wrong command \n");
				return 0;
			}

			if (isdigit(argv[i][0])) {
				sprintf(fileInputs[fileIndex++], "ScanInput_%s", argv[i]);
			} else {
				strcpy(fileInputs[fileIndex++], argv[i]);
			}

		}

	}

	if (fileIndex < 1) {
		printf("Scan File Inputs did not entered\n");
		return false;
	}

	if (!(sanityID < SCANTYPE_MAX && seqID == SEQTYPE_NONE && sanityID != modeID)) {
		sanityID = SCANTYPE_MAX;
	}

	printf("Test is running with %d repeats\n", repeat);

	return process(fileInputs);
}



