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

	printf("-------------- Bench Tool -----------------              \n");
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

	bool timer_enabled = true;
	bool print_enabled = false;

	int repeat = 1;

	if (argc <= 1) {
		printHelp();
		return 0;
	}

	int filtered_argc = 0;
	char filtered_argv[ARGV_MAX][ARGV_LENGTH];

	setenv("LC_NUMERIC", "C", 1);
        
	CPU *cpu = new CPU();

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
			} else if (!strcmp(argv[i], "smart1")) {
				powerMode = POWER_SMARTCALC;
			} else if (!strcmp(argv[i], "smart2")) {
				powerMode = POWER_SMARTREAL;
			}

		} else if (!strcmp (argv[i], "-r")) {

			if (isdigit(argv[++i][0])) {
				repeat = atoi(argv[i]);
				if (repeat < 1 || repeat > 1000) {
					repeat = 1;
				}
			}

		} else if (!strcmp (argv[i], "-p")) {

			if (!strcmp(argv[++i], "on")) {
				print_enabled = true;
			}

		} else if (!strcmp (argv[i], "-t")) {

			if (!strcmp(argv[++i], "off")) {
				timer_enabled = false;
			}

		} else if (!strcmp (argv[i], "-q")) {
			gpu->platformQuery();
			delete cpu;
			delete gpu;
			return 0;

		} else {

			strcpy(filtered_argv[filtered_argc], argv[i]);
			filtered_argc++;
		}
	}

	Power *power = Power::newInstance(powerMode, cpu->powerCoreCount);

	App *app = App::newInstance(appMode, cpu, gpu, power);

	app->setRepeat(repeat);
	app->setTimerState(timer_enabled);
	app->setPrintState(print_enabled);

	app->run(filtered_argc, filtered_argv);

	delete cpu;
	delete gpu;
	delete power;
	return 0;

}
