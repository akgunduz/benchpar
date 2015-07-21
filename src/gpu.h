//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __GPU_H_
#define __GPU_H_

#include "common.h"

class GPU {

	bool enabled;

	cl_device_id *clDevices;

	cl_uint numPlatforms;
	cl_platform_id* platforms;
	cl_uint numDevices;

	cl_program createBuildProgramFromFile(cl_context context,
			cl_device_id device,
			const char* buildOptions,
			const char* fileName);

	cl_program createBuildProgram(cl_context context,
			cl_device_id device,
			const char* buildOptions,
			const char* source);

public:

	cl_program clProgram;

	cl_context clGPUContext;

	cl_command_queue clCommandQue;

	size_t  localWorkSize[2], globalWorkSize[2];

	void checkErr(const char *, cl_int);

	void platformQuery();
	bool platformInit();
	bool platformDeInit();

	bool getEnabled();

	void getInfo();

	GPU();
	~GPU();
};
#endif
