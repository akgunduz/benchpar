//
// Created by Haluk AKGUNDUZ on 13/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __function_H_
#define __function_H_

#include "gpu.h"

class Function;

typedef bool (Function::*fFuncs)(Function *, GPU *);

class FuncList {

public:

	const char *id;
	const char *kernelid;
	int divider;
	fFuncs f;

	FuncList(){}
	void set(const char *id, const char *kernelid, int divider, fFuncs func) {
		this->id = id;
		this->kernelid = kernelid;
		this->divider = divider;
		this->f = func;
	};
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
