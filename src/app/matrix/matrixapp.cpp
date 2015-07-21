//
//  matrix.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "matrixapp.h"

Matrix* MatrixApp::process(Matrix *A, Matrix *B, int modeID, bool print, int repeat) {

	if (!A->check(B)) {
		return NULL;
	}

	Matrix* calculated = new Matrix(A->getRow(), B->getCol());

	if (power != NULL) {
		power->read_before();
	}

	Timer t1, t2;
	double t_min = 10000000.0f, t_max = 0.0f, t_avg = 0.0f;

	t1.snapshot();

	for (int i = 0; i < repeat; i++) {

		t2.snapshot();
		if (!(A->*(A->multipliers[modeID].f))(B, calculated, gpu)) {
			printf("\nMultiplication Method: %s failed\n", A->multipliers[modeID].id);
			delete calculated;
			return NULL;
		}
		double t2_diff = t2.getdiff();
		if (t_min > t2_diff) {
			t_min = t2_diff;
		}
		if (t2_diff > t_max) {
			t_max = t2_diff;
		}

		t_avg += t2_diff;
	}

	double t1_diff = t1.getdiff();
	double consumed = 0;

	if (power != NULL) {
		consumed = power->read_after();
	}

	if (print) {
		calculated->printOut();
	}

	t_avg /= repeat;

	printf("\nMultiplication Method: %s \n", A->multipliers[modeID].id);
	printf("Multiplication Time: %.3lf seconds!!!\n", t1_diff);
	printf("Min Time: %.3lfs, Max Time: %.3lfs, Avg Time: %.3lfs\n", t_min, t_max, t_avg);

	if (power != NULL && power->getMode() != POWER_OFF) {
		printf("Power -> Method: %s, Consumed: %.3lf Joules!!!\n",
				power->getName(), consumed);
	}

	return calculated;
}


bool MatrixApp::init(int argc, const char argv[][ARGV_LENGTH]) {

	enum SEQTYPE seqID = SEQTYPE_NONE;
	enum MULTYPE modeID = MULTYPE_CPU_STD;
	enum MULTYPE sanityID = MULTYPE_MAX;

	char fileInputs[2][255];
	int fileIndex = 0;

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

			if (fileIndex == 2) {
				printf("Wrong command \n");
				return 0;
			}

			if (isdigit(argv[i][0])) {
				sprintf(fileInputs[fileIndex++], "MatrixInput_%s", argv[i]);
			} else {
				strcpy(fileInputs[fileIndex++], argv[i]);
			}

		}

	}

	if (fileIndex < 2) {
		printf("Matrix File Inputs did not entered\n");
		return 0;
	}

	if (!(sanityID < MULTYPE_MAX && seqID == SEQTYPE_NONE && sanityID != modeID)) {
		sanityID = MULTYPE_MAX;
	}

	printf("Test is running with %d repeats\n", repeat);

	Timer t;

	Matrix *A, *B;

	t.snapshot();

	try {

		A = new Matrix(fileInputs[0]);

	} catch(const std::runtime_error e) {

		printf("Matrix A could not created!!!, Exception : %s\n", e.what());
		return 0;
	}

	try {

		B = new Matrix(fileInputs[1]);

	} catch(const std::runtime_error e) {

		printf("Matrix B could not created!!!, Exception : %s\n", e.what());
		delete A;
		return 0;
	}

	if (seqID == SEQTYPE_NONE) {

		Matrix *C = process(A, B, modeID, print_enabled, repeat);
		if (C != NULL && sanityID < MULTYPE_MAX) {
			Matrix *D = process(A, B, sanityID, false, 1);

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
			process(A, B, i, false, 1);
		}
	}

	double t_diff = t.getdiff();
	printf("\nTotal Time: %.3lf seconds!!!\n", t_diff);

	delete A;
	delete B;

	return true;
}
