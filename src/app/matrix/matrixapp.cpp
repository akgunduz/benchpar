//
//  matrix.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "matrixapp.h"

MatrixApp::MatrixApp(CPU *c, GPU *g, Power *pw) :
		App(c, g, pw) {

	loadGPUKernel();
}

MatrixApp::~MatrixApp() {

	unLoadGPUKernel();
}

int MatrixApp::getFuncModeCount(FUNCTYPE functype) {

	switch(functype) {
		case FUNCTYPE_CPU:
			return MULTYPE_GPU_STD;
		case FUNCTYPE_GPU:
			return MULTYPE_MAX - MULTYPE_GPU_STD;
		case FUNCTYPE_ALL:
			return MULTYPE_MAX;
	}

	return 0;
}

bool MatrixApp::loadGPUKernel() {

	if (!gpu->getEnabled()) {
		return false;
	}

	char file[PATH_LENGTH];

	sprintf(file, "%s/matrix.cl", getcwd(NULL, 0));

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

void MatrixApp::unLoadGPUKernel() {

	if (gpu_kernel_loaded) {
		clReleaseProgram(gpu->clProgram);
		clReleaseCommandQueue(gpu->clCommandQue);
	}
}

Matrix* MatrixApp::calculate(Matrix *A, Matrix *B, int modeID, int repeat) {

	if (!A->check(B)) {
		return NULL;
	}

	Matrix* calculated = new Matrix(A->getRow(), B->getCol());

	Timer t;
	double t_min = 10000000.0f, t_max = 0.0f, t_total = 0.0f, consumed = 0.0f;

	if (power != NULL) {
		power->read_before();
	}

	for (int i = 0; i < repeat; i++) {

		t.snapshot();

		if (!(A->*(A->multipliers[modeID].f))(B, calculated, gpu)) {
			printOut("\nMultiplication Method: %s failed\n", A->multipliers[modeID].id);
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

	printOut("\nMultiplication Method: %s \n", A->multipliers[modeID].id);
	printOut("Multiplication Time: %.3lfms!!!\n", t_total);
	printOut("Min Time: %.3lfms, Max Time: %.3lfms, Avg Time: %.3lfms\n", t_min, t_max, t_total / repeat);

	if (power != NULL && power->getMode() != POWER_OFF) {
		printOut("Power -> Method: %s, Consumed: %.3lf Joules!!!\n",
				power->getName(), consumed);
	}

	return calculated;
}

bool MatrixApp::process(char fileInput[255]) {

	Timer t;

	Matrix *A, *B;

	t.snapshot();

	printOut("\n=========================================\n");
	printOut("Loading Matrix Input File : %s ... \n", fileInput);

	try {

		A = new Matrix(fileInput, &B);

	} catch(const std::runtime_error e) {

		printOut("Matrix A & B could not created!!!, Exception : %s\n", e.what());
		return false;
	}

	printOut("Calculate Started... %dx%d with %dx%d\n", A->getRow(), A->getCol(), B->getRow(), B->getCol());

	if (seqID == SEQTYPE_NONE) {

		Matrix *C = calculate(A, B, modeID, repeat);
		if (C != NULL && sanityID != INVALID_SANITY) {
			Matrix *D = calculate(A, B, sanityID, 1);

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
				startIndex = MULTYPE_CPU_STD;
				count = MULTYPE_MAX - MULTYPE_CPU_STD;
				break;
			case SEQTYPE_CPU:
				startIndex = MULTYPE_CPU_STD;
				count = MULTYPE_GPU_STD - MULTYPE_CPU_STD;
				break;
			case SEQTYPE_GPU:
				startIndex = MULTYPE_GPU_STD;
				count = MULTYPE_MAX - MULTYPE_GPU_STD;
				break;
		}

		for (int i = startIndex; i < startIndex + count; i++) {
			calculate(A, B, i, repeat);
		}
	}

	double t_diff = t.getdiff();
	printOut("Total Time: %.3lf ms!!!\n\n", t_diff);

	delete A;
	delete B;

	return true;
}

bool MatrixApp::processDir(const char path[255]) {

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

		if (strncmp(ent->d_name, "MatrixInput", 11) != 0) {
			continue;
		}

		sprintf(fileInput, "%s/%s", path, ent->d_name);
		bool status = process(fileInput);
		if (!status) {
			return false;
		}
	}

	return true;
}

bool MatrixApp::processList(char fileInputs[][255], int size) {

	for (int i = 0; i < size; i++) {

		bool status = process(fileInputs[i]);
		if (!status) {
			return false;
		}
	}

	return true;
}

bool MatrixApp::run(int argc, const char argv[][ARGV_LENGTH]) {

	char fileInputs[MAX_FILE_COUNT][255];
	int fileID = 0;
	int fileIndex = 0;
	bool status;

	for (int i = 0; i < argc; i++) {

		if (!strcmp (argv[i], "-c")) {

			if (i + 3 >= argc) {
				printf("Matrix Creator needs printID, row and column values \n");
				return 0;
			}

			return creator((unsigned)atoi(argv[i+1]), (unsigned)atoi(argv[i+2]), (unsigned)atoi(argv[i+3]));

		} else {

			if (fileIndex == MAX_FILE_COUNT) {
				continue;
			}

			if (isdigit(argv[i][0])) {
				fileID = atoi(argv[i]);
				if (fileID == 0 || fileIndex > 0) {
					fileID = 0xFF;
				}
				sprintf(fileInputs[fileIndex++], "matrix/MatrixInput_%s", argv[i]);

			} else {
				fileID = 0xFF;
				sprintf(fileInputs[fileIndex++], "matrix/%s", argv[i]);
			}
		}

	}

	if (!printStart("matrix/Matrix", fileID)) {
		return false;
	}

	if (fileID == 0) {

		printOut("Test is running in Directory Mode with %d repeats\n", repeat);

		status = processDir("matrix");

	} else {

		printOut("Test is running with %d repeats\n", repeat);

		status = processList(fileInputs, fileIndex);
	}

	if (debugFile) {
		fclose(debugFile);
	}

	return status;
}

bool MatrixApp::creator(uint32_t printID, uint32_t row, uint32_t col) {

	Matrix *A = new Matrix(row, col, true);

	Matrix *B = new Matrix(col, row, true);

	A->printToFile(printID, B);

	delete A;
	delete B;
	return true;
}
