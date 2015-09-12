//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __scanapp_H_
#define __scanapp_H_


#include "app.h"
#include "scan.h"

class ScanApp : public App  {

	enum SEQTYPE seqID = SEQTYPE_NONE;
	enum SCANTYPE modeID = SCANTYPE_CPU_STD;
	enum SCANTYPE sanityID = SCANTYPE_MAX;

public:

	ScanApp(CPU *c, GPU *g, Power *pw);
	~ScanApp();

	virtual bool loadGPUKernel();
	virtual void unLoadGPUKernel();

	Scan* calculate(Scan *A, int modeID, bool print, int repeat);

	bool process(char fileInputs[][255]);
	bool processDir(const char path[255]);

	virtual bool run(int argc, const char argv[][ARGV_LENGTH]);
};


#endif //__scanapp_H_
