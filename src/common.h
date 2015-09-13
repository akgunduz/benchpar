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
#include <stdarg.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif


#define PATH_LENGTH 255
#define TILESIZE 32
#define ALIGNMENT 16
#define INVALID_SANITY 0xFFFF
#define MAX_FILE_COUNT 10
#define MAX_TIMEARRAY_COUNT 1000

enum SEQTYPE {
	SEQTYPE_NONE,
	SEQTYPE_CPU,
	SEQTYPE_GPU,
	SEQTYPE_ALL,
	SEQTYPE_MAX
};

enum FUNCTYPE {
	FUNCTYPE_CPU,
	FUNCTYPE_GPU,
	FUNCTYPE_ALL
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
