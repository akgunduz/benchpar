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

int ConvApp::getFuncModeCount(FUNCTYPE functype) {

	switch(functype) {
		case FUNCTYPE_CPU:
			return CONVTYPE_GPU_STD;
		case FUNCTYPE_GPU:
			return CONVTYPE_MAX - CONVTYPE_GPU_STD;
		case FUNCTYPE_ALL:
			return CONVTYPE_MAX;
	}

	return 0;
}

bool ConvApp::loadGPUKernel() {

	if (!gpu->getEnabled()) {
		return false;
	}

	char file[PATH_LENGTH];

	sprintf(file, "%s/conv.cl", getcwd(NULL, 0));

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

Conv* ConvApp::calculate(Conv *A, int modeID, int repeat) {

	if (!A->check()) {
		return NULL;
	}

#ifdef ENABLE_TIMEARRAY
	double sTime[MAX_TIMEARRAY_COUNT];
#endif

	Conv* calculated = new Conv(A->getRow(), A->getCol(), A->getFilter(), A->getFilterLength());

	printOut("\nConv Method: %s \n", A->convFuncs[modeID].id);

	Timer t;
	double t_min = 10000000.0f, t_max = 0.0f, t_total = 0.0f, consumed = 0.0f;

	if (power != NULL) {
		power->read_before();
	}

	for (int i = 0; i < repeat; i++) {

		t.snapshot();

		if (!(A->*(A->convFuncs[modeID].f))(calculated, gpu)) {
			printOut("\nConv Method: %s failed\n", A->convFuncs[modeID].id);
			delete calculated;
			return NULL;
		}
		double t_diff = t.getdiff();

		if (i == 0 && repeat > 1) {
			continue;
		}

		if (t_min > t_diff) {
			t_min = t_diff;
		}
		if (t_diff > t_max) {
			t_max = t_diff;
		}

		t_total += t_diff;

#ifdef ENABLE_TIMEARRAY
		if (i < MAX_TIMEARRAY_COUNT) {
			sTime[i] = t_diff;
		}
#endif
	}

	if (power != NULL) {
		consumed = power->read_after();
	}

#ifdef ENABLE_TIMEARRAY
	printOut("Time values : ");
	for (int i = 1; i < repeat; i++) {
		if (i < MAX_TIMEARRAY_COUNT) {
			printOut("%.3lf, ", sTime[i]);
		}
	}
#endif

	printOut("Conv Time: %.3lfms!!!\n", t_total);
	printOut("Min Time: %.3lfms, Max Time: %.3lfms, Avg Time: %.3lfms\n", t_min, t_max, t_total / repeat);

	if (power != NULL && power->getMode() != POWER_OFF) {
		printOut("Power -> Method: %s, Consumed: %.3lf Joules!!!\n",
				power->getName(), consumed);
	}

	return calculated;
}

uint32_t ConvApp::processFilter(char fileInput[255], float **filter) {

	uint32_t filter_length = 0;

	if (strcmp(fileInput, "") == 0) {

		filter_length = FILTER_LENGTH;

		int res = posix_memalign((void**)filter, ALIGNMENT, filter_length * sizeof(float));
		if (res != 0) {
			printOut("Alloc failed! : %d\n", errno);
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
			printOut("Alloc failed! : %d\n", errno);
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

bool ConvApp::process(char fileInput[255], char filterInput[255]) {

	Timer t;

	Conv *A;

	t.snapshot();

	printOut("\n=========================================\n");
	printOut("Loading Conv Input File : %s ... \n", fileInput);

	try {

		float *filter;
		uint32_t filter_length;

		filter_length = processFilter(filterInput, &filter);
		A = new Conv(fileInput, filter, filter_length);

	} catch(const std::runtime_error e) {

		printOut("Scan A could not created!!!, Exception : %s\n", e.what());
		return false;
	}

	printOut("Calculate Started... %dx%d\n", A->getRow(), A->getCol());

	if (seqID == SEQTYPE_NONE) {

		Conv *C = calculate(A, modeID, repeat);
		if (C != NULL && sanityID != INVALID_SANITY) {
			Conv *D = calculate(A, sanityID, 1);

			if (D != NULL) {
				if (C->compare(D)) {
					printOut("Sanity check passed\n");
				} else {
					printOut("Sanity check failed\n");
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
			calculate(A, i, repeat);
		}
	}

	double t_diff = t.getdiff();
	printOut("\nTotal Time: %.3lf ms!!!\n\n", t_diff);

	delete A;

	return true;
}

bool ConvApp::processDir(const char path[255], char filterInput[255]) {

	char fileInput[255];

	struct dirent *ent;

	DIR *dir = opendir(path);
	if (dir == nullptr) {
		printOut("Directory : %s could not opened\n err: %d", path, errno);
		return false;
	}

	while((ent = readdir(dir)) != nullptr) {

		if (ent->d_type != DT_REG) {
			continue;
		}

		if (strncmp(ent->d_name, "ConvInput", 9) != 0) {
			continue;
		}

		sprintf(fileInput, "%s/%s", path, ent->d_name);
		bool status = process(fileInput, filterInput);
		if (!status) {
			return false;
		}

	}

	return true;
}

bool ConvApp::processList(char fileInputs[][255], char filterInput[255], int size) {

	for (int i = 0; i < size; i++) {

		bool status = process(fileInputs[i], filterInput);
		if (!status) {
			return false;
		}
	}

	return true;
}

bool ConvApp::run(int argc, const char argv[][ARGV_LENGTH]) {

	char filterInput[255];
	char fileInputs[MAX_FILE_COUNT][255];
	int fileID = 0;
	int fileIndex = 0;
	bool status;

	strcpy(filterInput, "");

	for (int i = 0; i < argc; i++) {

		if (!strcmp (argv[i], "-c")) {

			if (i + 3 >= argc) {
				printf("Conv Creator needs printID, size values \n");
				return 0;
			}

			return creator((unsigned)atoi(argv[i+1]), (unsigned)atoi(argv[i+2]), (unsigned)atoi(argv[i+3]));

		} else if (!strcmp (argv[i], "-f")) {

			if (isdigit(argv[++i][0])) {
				sprintf(filterInput, "conv/FilterInput_%s", argv[i]);
			} else {
				sprintf(filterInput, "conv/%s", argv[i]);
			}

		} else {

			if (fileIndex == MAX_FILE_COUNT) {
				continue;
			}

			if (isdigit(argv[i][0])) {
				fileID = atoi(argv[i]);
				if (fileID == 0 || fileIndex > 0) {
					fileID = 0xFF;
				}
				sprintf(fileInputs[fileIndex++], "conv/ConvInput_%s", argv[i]);

			} else {
				fileID = 0xFF;
				sprintf(fileInputs[fileIndex++], "conv/%s", argv[i]);
			}

		}
	}

	if (!printStart("conv/Conv", fileID)) {
		return false;
	}

	if (fileID == 0) {

		printOut("Test is running in Directory Mode with %d repeats\n", repeat);

		status = processDir("conv", filterInput);

	} else {

		printOut("Test is running with %d repeats\n", repeat);

		status = processList(fileInputs, filterInput, fileIndex);
	}

	if (debugFile) {
		fclose(debugFile);
	}

	return status;
}


bool ConvApp::creator(uint32_t printID, uint32_t row, uint32_t col) {

	Conv *A = new Conv(row, col, NULL, 0, true);
	A->printToFile(printID);
	delete A;
	return true;
}
