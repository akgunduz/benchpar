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

bool Timer::enabled = true;

void printHelp() {

	printf("---------- Bench Tool        ----------                  \n");
	printf("-----------------Howto ----------------                  \n");
	printf("There are 3 main running modes:                        \n\n");
	printf("1. Matrix Create Mode:                                   \n");
	printf("Usage: -c printID row column                             \n");
	printf("Sample: -c 10 64 1024                                    \n");
	printf("Creates a MatrixInput_10 file with a 64x1024 matrix in it\n");
	printf("\n2. Platform Query Mode:                                \n");
	printf("Usage: -q                                                \n");
	printf("Queries the platform and lists on the console            \n");
	printf("\n3. Matrix Calculation Mode:(default)                   \n");
	printf("Usage: fileInput1 fileInput2                             \n");
	printf("Parameters:                                              \n");
	printf("Timer:  -t off/on         default:on                     \n");
	printf("Print:  -p off/on         default:off                    \n");
	printf("Repeat: -r (1-1000)       default:1                      \n");
	printf("Power:  -o off/msr/perf   default:off                    \n");
	printf("Sanity: -s (0-6)          default:0                      \n");
	printf("Method: -m (0-6)/a/c/g    default:0                      \n");
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
	int powerCoreCount = 0;

	if (argc <= 1) {
		printHelp();
		return 0;
	}

        setenv("LC_NUMERIC", "C", 1);
        
	CPU *cpu = new CPU();
	if (cpu->type == PROC_INTEL) {
		powerMode = POWER_MSR;
		powerCoreCount = cpu->isIntelSeperateCorePower() ? cpu->coreCount : 1;
	} else if (cpu->type == PROC_ARM) {
		powerMode = POWER_INA;
		powerCoreCount = cpu->coreCount;
	}

	GPU *gpu = new GPU();

	for (int i = 1; i < argc; i++) {

		if (!strcmp(argv[i], "-a")) {

			if (!strcmp(argv[++i], "m")) {
				appMode = MATRIX_MODE;
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
			} else if (!strcmp(argv[i], "smart")) {
				powerMode = POWER_SMART;
			}

		} else if (!strcmp (argv[i], "-q")) {
			gpu->platformQuery();
			delete cpu;
			delete gpu;
			return 0;

		}
	}

	Power *power = Power::newInstance(powerMode, powerCoreCount);

	App *app = App::newInstance(appMode, cpu, gpu, power);
	app->init(argc, argv);

	delete cpu;
	delete gpu;
	delete power;
	return 0;

}
