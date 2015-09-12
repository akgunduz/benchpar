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

Matrix* MatrixApp::calculate(Matrix *A, Matrix *B, int modeID, bool print, int repeat) {

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
			printf("\nMultiplication Method: %s failed\n", A->multipliers[modeID].id);
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

	printf("\nMultiplication Method: %s \n", A->multipliers[modeID].id);
	printf("Multiplication Time: %.3lf seconds!!!\n", t_total);
	printf("Min Time: %.3lfs, Max Time: %.3lfs, Avg Time: %.3lfs\n", t_min, t_max, t_total / repeat);

	if (power != NULL && power->getMode() != POWER_OFF) {
		printf("Power -> Method: %s, Consumed: %.3lf Joules!!!\n",
				power->getName(), consumed);
	}

	return calculated;
}

bool MatrixApp::process(char fileInputs[][255]) {

	Timer t;

	Matrix *A, *B;

	t.snapshot();

	printf("\n=========================================\n");
	printf("Loading Matrix Input Files from Storage... \n");

	try {

		A = new Matrix(fileInputs[0]);

	} catch(const std::runtime_error e) {

		printf("Matrix A could not created!!!, Exception : %s\n", e.what());
		return false;
	}

	try {

		B = new Matrix(fileInputs[1]);

	} catch(const std::runtime_error e) {

		printf("Matrix B could not created!!!, Exception : %s\n", e.what());
		delete A;
		return false;
	}

	printf("Calculate Started... %dx%d with %dx%d\n", A->getRow(), A->getCol(), B->getRow(), B->getCol());

	if (seqID == SEQTYPE_NONE) {

		Matrix *C = calculate(A, B, modeID, print_enabled, repeat);
		if (C != NULL && sanityID < MULTYPE_MAX) {
			Matrix *D = calculate(A, B, sanityID, false, 1);

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
			calculate(A, B, i, false, 1);
		}
	}

	double t_diff = t.getdiff();
	printf("Total Time: %.3lf seconds!!!\n\n", t_diff);

	delete A;
	delete B;

	return true;
}

bool MatrixApp::processDir(const char path[255]) {

	char fileInputs[2][255];

	struct dirent *ent;

	DIR *dir = opendir(path);
	if (dir == nullptr) {
		printf ("Directory : %s could not opened\n err: %d", path, errno);
		return false;
	}

	int fileIndex = 0;

	while((ent = readdir(dir)) != nullptr) {

		if (ent->d_type != DT_REG) {
			continue;
		}

		if (strncmp(ent->d_name, "MatrixInput", 11) != 0) {
			continue;
		}

		if (fileIndex == 0) {
			sprintf(fileInputs[0], "%s/%s", path, ent->d_name);
			fileIndex = 1;

		} else {
			sprintf(fileInputs[1], "%s/%s", path, ent->d_name);
			fileIndex = 0;
			bool status = process(fileInputs);
			if (!status) {
				return false;
			}
		}
	}

	return true;
}

bool MatrixApp::run(int argc, const char argv[][ARGV_LENGTH]) {

	char fileInputs[2][255];
	int fileIndex = 0;
	bool dirMode = true;

	for (int i = 0; i < argc; i++) {

		if (!strcmp (argv[i], "-c")) {

			if (i + 3 >= argc) {
				printf("Matrix Creator needs printID, row and column values \n");
				return 0;
			}

			Matrix *A = new Matrix((unsigned)atoi(argv[i+2]), (unsigned)atoi(argv[i+3]), true);
			A->printToFile((unsigned)atoi(argv[i+1]));
			delete A;
			return true;

		} else if (!strcmp (argv[i], "-m")) {

			if (i + 1 >= argc) {
				printf("Mode Setting needs modeID \n");
				return 0;
			}

			if (isdigit(argv[++i][0])) {
				modeID = (MULTYPE) atoi(argv[i]);
				if (modeID >= MULTYPE_MAX) {
					modeID = MULTYPE_CPU_STD;
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
				sanityID = (MULTYPE) atoi(argv[i]);
			}

		} else {

			dirMode = false;

			if (fileIndex == 2) {
				continue;
			}

			if (isdigit(argv[i][0])) {
				sprintf(fileInputs[fileIndex++], "matrix/MatrixInput_%s", argv[i]);
			} else {
				sprintf(fileInputs[fileIndex++], "matrix/%s", argv[i]);
			}
		}

	}

	if (!(sanityID < MULTYPE_MAX && seqID == SEQTYPE_NONE && sanityID != modeID)) {
		sanityID = MULTYPE_MAX;
	}

	if (dirMode) {

		printf("Test is running in Directory Mode with %d repeats\n", repeat);

		return processDir("matrix");

	} else {

		if (fileIndex < 2) {
			printf("Matrix File Inputs did not entered\n");
			return false;
		}

		printf("Test is running with %d repeats\n", repeat);

		return process(fileInputs);
	}
}
