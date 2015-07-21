//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __APP_H_
#define __APP_H_

#include "common.h"
#include "power.h"
#include "gpu.h"
#include "matrix.h"

#define ARGV_LENGTH 30
#define ARGV_MAX 20

enum APP_MODES {
	MATRIX_MODE,
};


class App {

protected:

	Power *power;
	GPU *gpu;
	CPU *cpu;

	bool timer_enabled;
	bool print_enabled;

	int repeat;

protected:

	App(CPU *c, GPU *g, Power *pw):
			cpu(c), gpu(g), power(pw) {

		timer_enabled = true;
		print_enabled = false;

		repeat = 1;
	};

public :

	static App* newInstance(APP_MODES, CPU*, GPU*, Power*);

	void setTimerState(bool);
	void setPrintState(bool);

	void setRepeat(int);

	virtual bool init(int argc, const char argv[][ARGV_LENGTH]) = 0;

};

#endif
