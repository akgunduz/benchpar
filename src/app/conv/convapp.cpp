//
//  scanapp.cpp
//  ScanCalculator
//
//  Created by Haluk AKGUNDUZ on 03/09/15.
//  Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//

#include "convapp.h"

ConvApp::ConvApp(CPU *c, GPU *g, Power *pw) :
		App(c, g, pw) {

	loadGPUKernel();
}

ConvApp::~ConvApp() {

	unLoadGPUKernel();
}

bool ConvApp::loadGPUKernel() {

	if (!gpu->getEnabled()) {
		return false;
	}

	char file[PATH_LENGTH];

	sprintf(file, "%s/convolutionSeparable.cl", getcwd(NULL, 0));

	char buildOptions[2048];
	sprintf(buildOptions, "-cl-fast-relaxed-math -cl-mad-enable");

	bool res = gpu->createBuildProgramFromFile(0, buildOptions, file);

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

	gpu_kernel_loaded = true;

	return true;
}

void ConvApp::unLoadGPUKernel() {

	if (gpu_kernel_loaded) {
		clReleaseProgram(gpu->clProgram);
		clReleaseCommandQueue(gpu->clCommandQue);
	}
}

Conv* ConvApp::calculate(Conv *A, int modeID, bool print, int repeat) {

	if (!A->check()) {
		return NULL;
	}

	Conv* calculated = new Conv(A->getRow(), A->getCol(), A->getFilter(), A->getFilterLength());

	Timer t;
	double t_min = 10000000.0f, t_max = 0.0f, t_total = 0.0f, consumed = 0.0f;

	if (power != NULL) {
		power->read_before();
	}

	for (int i = 0; i < repeat; i++) {

		t.snapshot();

		if (!(A->*(A->convFuncs[modeID].f))(calculated, gpu)) {
			printf("\nConv Method: %s failed\n", A->convFuncs[modeID].id);
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

	printf("\nConv Method: %s \n", A->convFuncs[modeID].id);
	printf("nConv Time: %.3lf seconds!!!\n", t_total);
	printf("Min Time: %.3lfs, Max Time: %.3lfs, Avg Time: %.3lfs\n", t_min, t_max, t_total / repeat);

	if (power != NULL && power->getMode() != POWER_OFF) {
		printf("Power -> Method: %s, Consumed: %.3lf Joules!!!\n",
				power->getName(), consumed);
	}

	return calculated;
}

uint32_t ConvApp::processFilter(char fileInput[255], float **filter) {

	uint32_t filter_length = 0;

	if (strcmp(fileInput, "") == 0) {

		filter_length = 5;

		int res = posix_memalign((void**)filter, ALIGNMENT, filter_length * sizeof(float));
		if (res != 0) {
			printf("Alloc failed! : %d\n", errno);
			return 0;
		}

		(*filter)[0] = 1.1f / 11.0f;
		(*filter)[1] = 2.2f / 11.0f;
		(*filter)[2] = 4.4f / 11.0f;
		(*filter)[3] = 2.2f / 11.0f;
		(*filter)[4] = 1.1f / 11.0f;

	} else {

		FILE *fd = fopen(fileInput, "r");
		if (!fd) {
			throw std::runtime_error("File could not opened!");
		}

		int res = fscanf(fd, "%d", &filter_length);
		if (res == EOF) {
			fclose(fd);
			return 0;
		}

		res = posix_memalign((void**)filter, ALIGNMENT, filter_length * sizeof(float));
		if (res != 0) {
			printf("Alloc failed! : %d\n", errno);
			return 0;
		}

		for (int i = 0; i < filter_length; i++) {

			res = fscanf(fd, "%f,", &(*filter)[i]);
			if (res == EOF) {
				fclose(fd);
				return 0;
			}
		}

		fclose(fd);

	}

	return filter_length;
}

bool ConvApp::process(char fileInputs[][255]) {

	Timer t;

	Conv *A;

	t.snapshot();

	printf("Loading Scan Input Files from Storage... \n");

	try {

		float *filter;
		uint32_t filter_length;

		if (strncmp(fileInputs[0], "Filter", 6) == 0) {
			filter_length = processFilter(fileInputs[0], &filter);
			A = new Conv(fileInputs[1], filter, filter_length);

		} else {
			filter_length = processFilter(fileInputs[1], &filter);
			A = new Conv(fileInputs[0], filter, filter_length);
		}

	} catch(const std::runtime_error e) {

		printf("Scan A could not created!!!, Exception : %s\n", e.what());
		return false;
	}

	printf("Calculate Started... \n");

	if (seqID == SEQTYPE_NONE) {

		Conv *C = calculate(A, modeID, print_enabled, repeat);
		if (C != NULL && sanityID < CONVTYPE_MAX) {
			Conv *D = calculate(A, sanityID, false, 1);

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
				startIndex = CONVTYPE_CPU_STD;
				count = CONVTYPE_MAX - CONVTYPE_CPU_STD;
				break;
			case SEQTYPE_CPU:
				startIndex = CONVTYPE_CPU_STD;
				count = CONVTYPE_GPU_STD - CONVTYPE_CPU_STD;
				break;
			case SEQTYPE_GPU:
				startIndex = CONVTYPE_GPU_STD;
				count = CONVTYPE_MAX - CONVTYPE_GPU_STD;
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

bool ConvApp::run(int argc, const char argv[][ARGV_LENGTH]) {

	char fileInputs[2][255];
	int fileIndex = 0;
	strcpy(fileInputs[0], "");
	strcpy(fileInputs[1], "");
	bool filterEnabled = false;

	for (int i = 0; i < argc; i++) {

		if (!strcmp (argv[i], "-c")) {

			if (i + 2 >= argc) {
				printf("Conv Creator needs printID, size values \n");
				return 0;
			}

			Conv *A = new Conv((unsigned)atoi(argv[i+2]), (unsigned)atoi(argv[i+3]), NULL, 0, true);
			A->printToFile((unsigned)atoi(argv[i+1]));
			delete A;
			return true;

		} else if (!strcmp (argv[i], "-m")) {

			if (i + 1 >= argc) {
				printf("Mode Setting needs modeID \n");
				return 0;
			}

			if (isdigit(argv[++i][0])) {
				modeID = (CONVTYPE) atoi(argv[i]);
				if (modeID >= CONVTYPE_MAX) {
					modeID = CONVTYPE_CPU_STD;
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
				sanityID = (CONVTYPE) atoi(argv[i]);
			}

		} else if (!strcmp (argv[i], "-f")) {

			filterEnabled = true;
			if (isdigit(argv[++i][0])) {
				sprintf(fileInputs[fileIndex++], "FilterInput_%s", argv[i]);
			} else {
				strcpy(fileInputs[fileIndex++], argv[i]);
			}

		} else {

			if (fileIndex == 2) {
				printf("Wrong command \n");
				return 0;
			}

			if (isdigit(argv[i][0])) {
				sprintf(fileInputs[fileIndex++], "ConvInput_%s", argv[i]);
			} else {
				strcpy(fileInputs[fileIndex++], argv[i]);
			}

		}

	}

	if ((filterEnabled && fileIndex < 2) || (!filterEnabled && fileIndex < 1)) {
		printf("Conv File Inputs did not entered\n");
		return false;
	}

	if (!(sanityID < CONVTYPE_MAX && seqID == SEQTYPE_NONE && sanityID != modeID)) {
		sanityID = CONVTYPE_MAX;
	}

	printf("Test is running with %d repeats\n", repeat);

	return process(fileInputs);
}



