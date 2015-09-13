//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __scanapp_H_
#define __scanapp_H_


#include "app.h"
#include "scan.h"

class ScanApp : public App  {

public:

	ScanApp(CPU *c, GPU *g, Power *pw);
	~ScanApp();

	virtual bool loadGPUKernel();
	virtual void unLoadGPUKernel();

	Scan* calculate(Scan *A, int modeID, int repeat);

	virtual int getFuncModeCount(FUNCTYPE);
	virtual bool creator(uint32_t, uint32_t, uint32_t);

	bool process(char fileInput[255]);
	bool processDir(const char path[255]);
	bool processList(char fileInputs[][255], int size);

	virtual bool run(int argc, const char argv[][ARGV_LENGTH]);
};


#endif //__scanapp_H_
