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
	char kernelid[MAX_ARGUMENT][PATH_MAX];
	int argument[MAX_ARGUMENT];
        int argCount = 0;
	fFuncs f;

	FuncList();
        void set(const char *id, fFuncs func, int argCount, const char *kernelid[] = NULL, int argument[] = NULL);
};

class Function {

protected :
	size_t size;
	float *mem;
	size_t mem_size;

public:
	FuncList* funcList;

	Function();

	virtual void initFuncs() = 0;

};


#endif //__function_H_
