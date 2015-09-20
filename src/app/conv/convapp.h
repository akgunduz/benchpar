//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __convapp_H_
#define __convapp_H_


#include "app.h"
#include "conv.h"

class ConvApp : public App  {

public:

	ConvApp(CPU *c, GPU *g, Power *pw, const char *path);
	~ConvApp();

	virtual bool loadGPUKernel();
	virtual void unLoadGPUKernel();

	Conv* calculate(Conv *A, int modeID, int repeat);

	virtual int getFuncModeCount(FUNCTYPE);
	virtual bool creator(uint32_t, uint32_t, uint32_t);

	bool process(char fileInput[PATH_MAX], char filterInput[PATH_MAX]);
	bool processDir(const char path[PATH_MAX], char filterInput[PATH_MAX]);
	bool processList(char fileInputs[][PATH_MAX], char filterInput[PATH_MAX], int size);
	uint32_t processFilter(char fileInput[PATH_MAX], float **filter);

	virtual bool run(int argc, const char argv[][ARGV_LENGTH]);
};


#endif //__convapp_H_
