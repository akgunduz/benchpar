//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __scan_H_
#define __scan_H_

#include "function.h"

#define SIZE 512 * 1024
#define ALIGNMENT 16
#define RANGE 10

#define WORKGROUP_SIZE 256

#define ARRAY_LENGTH 256 * 1024 //(8 * WORKGROUP_SIZE)

#define MAX_BATCH_ELEMENTS   (64 * 1048576)
#define MIN_SHORT_ARRAY_SIZE (4)
#define MAX_SHORT_ARRAY_SIZE (4 * WORKGROUP_SIZE)
#define MIN_LARGE_ARRAY_SIZE (8 * WORKGROUP_SIZE)
#define MAX_LARGE_ARRAY_SIZE (4 * WORKGROUP_SIZE * WORKGROUP_SIZE)

enum SCANTYPE {
	SCANTYPE_CPU_STD,
#ifndef __ARM__
#ifdef __AVX__
	SCANTYPE_CPU_AVX,
#endif
	SCANTYPE_CPU_SSE,
	SCANTYPE_CPU_OMP_SSE,
	SCANTYPE_CPU_OMP_SSEp2_SSEp1,
#endif
	SCANTYPE_CPU_MAX,
#ifdef __OPENCL__
	SCANTYPE_GPU_STD = SCANTYPE_CPU_MAX,
	SCANTYPE_MAX,
#else
    SCANTYPE_MAX = SCANTYPE_CPU_MAX,
#endif
};

class Scan : public Function {

	bool allocMem(size_t size);

public:

	Scan(size_t size, GPU *, bool prepare = false);
	Scan(const char *path, GPU *);
	~Scan();

	virtual void initFuncs();

	bool printToFile(const char *, uint32_t);

	unsigned int iSnapUp(unsigned int, unsigned int);

	bool scanCPU_STD(Scan *, GPU *);

#ifndef __ARM__
#ifdef __AVX__
	bool scanCPU_AVX(Scan *, GPU *);
#endif
	bool scanCPU_SSE(Scan *, GPU *);
	bool scanCPU_OMP_SSE(Scan *, GPU *);
	bool scanCPU_OMP_SSEp2_SSEp1(Scan *, GPU *);
#endif

	bool scanGPU_STD(Scan *, GPU *);

	bool scanGPU(Scan *calculated, int type, GPU *);

	void create(size_t size);
	bool compare(Scan *);
	bool check();

	size_t getSize();
};


#endif //__scan_H_
