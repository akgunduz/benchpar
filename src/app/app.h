//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __APP_H_
#define __APP_H_

#include "common.h"
#include "power.h"
#include "gpu.h"

#define ARGV_LENGTH 30
#define ARGV_MAX 20

enum APP_MODES {
	MATRIX_MODE,
	SCAN_MODE,
	CONV_MODE,
};


class App {

protected:

	Power *power;
	GPU *gpu;
	CPU *cpu;

	bool print_enabled;
	int repeat;

	int seqID = SEQTYPE_NONE;
	int modeID = 0;
	int sanityID = INVALID_SANITY;

	bool gpu_kernel_loaded = false;

protected:

	App(CPU *c, GPU *g, Power *pw);

public :

	static App* newInstance(APP_MODES, CPU*, GPU*, Power*);

	void setPrintState(bool);

	void setRepeat(int);

	void setModes(int, int, int);

	virtual int getFuncModeCount(FUNCTYPE) = 0;
	virtual bool creator(uint32_t, uint32_t, uint32_t) = 0;

	virtual bool loadGPUKernel() = 0;
	virtual void unLoadGPUKernel() = 0;

	virtual bool run(int argc, const char argv[][ARGV_LENGTH]) = 0;

	virtual ~App();
};

#endif
