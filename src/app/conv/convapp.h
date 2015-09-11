//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __convapp_H_
#define __convapp_H_


#include "app.h"
#include "conv.h"

class ConvApp : public App  {

	enum SEQTYPE seqID = SEQTYPE_NONE;
	enum CONVTYPE modeID = CONVTYPE_CPU_STD;
	enum CONVTYPE sanityID = CONVTYPE_MAX;

public:

	ConvApp(CPU *c, GPU *g, Power *pw);
	~ConvApp();

	virtual bool loadGPUKernel();
	virtual void unLoadGPUKernel();

	Conv* calculate(Conv *A, int modeID, bool print, int repeat);

	bool process(char fileInputs[][255]);
	uint32_t processFilter(char fileInputs[255], float **filter);

	virtual bool run(int argc, const char argv[][ARGV_LENGTH]);
};


#endif //__convapp_H_
