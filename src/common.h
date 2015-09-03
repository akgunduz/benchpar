//
//  common.h
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#ifndef COMMON_H
#define COMMON_H

#include <omp.h>
#include <iostream>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdexcept>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif


#define PATH_LENGTH 255
#define TILESIZE 32

enum SEQTYPE {
	SEQTYPE_NONE,
	SEQTYPE_CPU,
	SEQTYPE_GPU,
	SEQTYPE_ALL,
	SEQTYPE_MAX
};


class Timer {

	double startTime;

	double getTime();

public:

	Timer() : startTime(0){}
	double snapshot();
	double getdiff();
};


#endif
