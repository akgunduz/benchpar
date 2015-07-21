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

enum APP_MODES {
	MATRIX_MODE,
};


class App {

protected:

	Power *power;
	GPU *gpu;
	CPU *cpu;

protected:

	App(CPU *c, GPU *g, Power *pw):
			cpu(c), gpu(g), power(pw) {};

public :

	static App* newInstance(APP_MODES, CPU*, GPU*, Power*);

	virtual bool init(int argc, const char *argv[]) = 0;

};

#endif
