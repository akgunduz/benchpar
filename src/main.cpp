//
//  main.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "matrix.h"
#include "cpu.h"
#include "power.h"
#include "app.h"

void printHelp() {

	printf("------------------- Bench Tool -------------------       \n");

	printf("Parameters:                                              \n");
	printf("App  :  -a m/s/c                         default:m       \n");
	printf("Print:  -p off/on                        default:off     \n");
	printf("Repeat: -r (1-2000000)                   default:1       \n");
	printf("Power:  -o off/msr/perf/ina/spc/spr      default:off     \n");

	printf("\n------------------- Matrix (m) -------------------     \n");

	printf("There are 3 main running modes:                        \n\n");
	printf("1. Matrix Create Mode:                                   \n");
	printf("Usage: -c printID row column                             \n");
	printf("Sample: -c 1 64 1024                                     \n");
	printf("Creates a MatrixInput_1 file with a 64x1024 matrix inside\n");
	printf("\n2. Platform Query Mode:                                \n");
	printf("Usage: -q                                                \n");
	printf("Queries the platform and lists on the console            \n");
	printf("\n3. Matrix Calculation Mode:(default)                   \n");
	printf("Usage: fileInput1 fileInput2 parameters...               \n");

	printf("Parameters:                                              \n");
	printf("Sanity: -s (0-6)                         default:disabled\n");
	printf("Method: -m (0-6)/a/c/g                   default:0       \n");
	printf("Methods:                                                 \n");
	printf("0 : MULTYPE_CPU_STD                                      \n");
	printf("1 : MULTYPE_CPU_TILED                                    \n");
	printf("2 : MULTYPE_CPU_TILED_BASIC                              \n");
	printf("3 : MULTYPE_CPU_TILED_OMP                                \n");
	printf("4 : MULTYPE_GPU_STD                                      \n");
	printf("5 : MULTYPE_GPU_VEC4                                     \n");
	printf("6 : MULTYPE_GPU_VEC8                                     \n");
	printf("a : Run All Modes Sequentially                           \n");
	printf("c : Run Only CPU Modes Sequentially                      \n");
	printf("g : Run Only GPU Modes Sequentially                      \n");

}

int main(int argc, const char *argv[]) {

	enum APP_MODES appMode = MATRIX_MODE;
	enum POWER_MODES powerMode = POWER_OFF;

	bool print_enabled = true;
	bool time_enabled = false;
	int repeat = 1;
	enum SEQTYPE seqID = SEQTYPE_NONE;
	int modeID = 0;
	int sanityID = INVALID_SANITY;

	int filtered_argc = 0;
	char filtered_argv[ARGV_MAX][ARGV_LENGTH];

	setenv("LC_NUMERIC", "C", 1);
        
	CPU *cpu = new CPU();
	GPU *gpu = new GPU();

	for (int i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-a")) {

			if (!strcmp(argv[++i], "m")) {
				appMode = MATRIX_MODE;
			} else if (!strcmp(argv[i], "s")) {
				appMode = SCAN_MODE;
			} else if (!strcmp(argv[i], "c")) {
				appMode = CONV_MODE;
			}

		} else if (!strcmp(argv[i], "-o")) {
			if (!strcmp(argv[++i], "off")) {
				powerMode = POWER_OFF;
			} else if (!strcmp(argv[i], "msr")) {
				powerMode = POWER_MSR;
			} else if (!strcmp(argv[i], "perf")) {
				powerMode = POWER_PERF;
			} else if (!strcmp(argv[i], "ina")) {
				powerMode = POWER_INA;
			} else if (!strcmp(argv[i], "spc")) {
				powerMode = POWER_SMARTCALC;
			} else if (!strcmp(argv[i], "spr")) {
				powerMode = POWER_SMARTREAL;
			}

		} else if (!strcmp (argv[i], "-m")) {

			if (i + 1 >= argc) {
				printf("Mode Setting needs modeID \n");
				return 0;
			}

			if (isdigit(argv[++i][0])) {
				modeID = atoi(argv[i]);

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
				sanityID = atoi(argv[i]);
			}

		} else if (!strcmp (argv[i], "-r")) {

			if (isdigit(argv[++i][0])) {
				repeat = atoi(argv[i]);
				if (repeat < 1 || repeat > 2000000000) {
					repeat = 1;
				}
			}

		} else if (!strcmp (argv[i], "-p")) {

			if (!strcmp(argv[++i], "off")) {
				print_enabled = false;
			}

		} else if (!strcmp (argv[i], "-t")) {

			if (!strcmp(argv[++i], "on")) {
				time_enabled = true;
			}

		} else if (!strcmp (argv[i], "-q")) {
			gpu->platformQuery();
			delete cpu;
			delete gpu;
			return 0;

		} else if (!strcmp (argv[i], "-h")) {
			printHelp();
			delete cpu;
			delete gpu;
			return 0;

		} else {
			strcpy(filtered_argv[filtered_argc++], argv[i]);
		}
	}

	Power *power = Power::newInstance(powerMode, cpu->powerCoreCount);

	App *app = App::newInstance(appMode, cpu, gpu, power, argv[0]);

	app->setModes(modeID, seqID, sanityID, print_enabled, time_enabled, repeat);

	app->run(filtered_argc, filtered_argv);

        delete app;
        delete power;
        delete gpu;
	delete cpu;

	return 0;

}
