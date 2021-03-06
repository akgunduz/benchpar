//
// Created by Haluk AKGUNDUZ on 13/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __function_H_
#define __function_H_

#include "gpu.h"

#define MAX_ARGUMENT 10

class Function;

typedef bool (Function::*fFuncs)(Function *, GPU *);

class FuncList {

public:

	const char *id;
	int argCount = 0;
	int argument[MAX_ARGUMENT];

	GPU *gpu;

	int kernelCount = 0;
#ifdef __OPENCL__
	cl_kernel kernels[MAX_ARGUMENT];
#endif
	fFuncs f;

	FuncList();
	~FuncList();
	void set(const char *id, fFuncs func, int kernelCount, int argCount, const char *kernelid[] = NULL, int argument[] = NULL);
	static FuncList* createArray(int size, GPU*);
};

class Function {

protected :
	size_t size;
	float *mem;
#ifdef __OPENCL__
	cl_mem buf_mem;
#endif
	size_t mem_size;

	GPU *gpu;

public:
	FuncList* funcList;

	Function(GPU *);
	virtual ~Function();

	void consoleOut(int, int);

	virtual void initFuncs() = 0;

};


#endif //__function_H_
