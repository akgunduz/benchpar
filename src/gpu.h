//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __GPU_H_
#define __GPU_H_

#include "common.h"

class GPU {

	bool enabled;

public:


#ifdef __OPENCL__
	cl_uint numPlatforms;
	cl_platform_id* platforms;
	cl_uint numDevices;
	cl_device_id *clDevices;

	cl_program clProgram;

	cl_context clGPUContext;

	cl_command_queue clCommandQue;

#endif
	void checkErr(const char *, int);

	void platformQuery();
	bool platformInit();
	bool platformDeInit();

	bool getEnabled();

	void getInfo();

	bool createCommandQueue(int deviceIndex, unsigned long);

	bool createBuildProgramFromFile(
			int deviceIndex,
			const char* buildOptions,
			const char* fileName);

	bool createBuildProgram(int deviceIndex,
			const char* buildOptions,
			const char* source);

	GPU();
	~GPU();
};
#endif
