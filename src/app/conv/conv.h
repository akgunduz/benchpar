//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __conv_H_
#define __conv_H_

#include "function.h"

#define HEIGHT 1024
#define WIDTH 1024
#define FILTER_LENGTH 5
#define RANGE 10

#define KERNEL_RADIUS 2

#define ROWS_BLOCKDIM_X 4//32
#define ROWS_BLOCKDIM_Y 4//8
#define COLUMNS_BLOCKDIM_X 4//32
#define COLUMNS_BLOCKDIM_Y 4
#define ROWS_RESULT_STEPS 4
#define ROWS_HALO_STEPS 1
#define COLUMNS_RESULT_STEPS 8//4
#define COLUMNS_HALO_STEPS 1

#define KERNEL_LENGTH (2 * KERNEL_RADIUS + 1)

enum CONVTYPE {

	CONVTYPE_CPU_STD,
	CONVTYPE_CPU_OMP,
	CONVTYPE_CPU_MAX,
#ifdef __OPENCL__
	CONVTYPE_GPU_STD = CONVTYPE_CPU_MAX,
	CONVTYPE_GPU_VEC4,
	CONVTYPE_GPU_COMB,
	CONVTYPE_MAX,
#else
	CONVTYPE_MAX = CONVTYPE_CPU_MAX,
#endif
	
};

class Conv : public Function {

	uint32_t row;
	uint32_t col;


	float *temp;
#ifdef __OPENCL__
	cl_mem buf_temp;
#endif
	float *filter;
	uint32_t filter_length;

	bool allocMem(uint32_t row, uint32_t col);

public:

	Conv(uint32_t row, uint32_t col, GPU *, float *filter, uint32_t filter_length, bool prepare = false);
	Conv(std::string path, GPU *, float *filter, uint32_t filter_length);
	~Conv();

	virtual void initFuncs();

	bool printToFile(const char *, uint32_t);

	bool convCPU_STD(Conv *, GPU *);
	bool convCPU_OMP(Conv *, GPU *);
#ifdef __OPENCL__
	bool convGPU_STD(Conv *, GPU *);
	bool convGPU_VEC4(Conv *, GPU *);
	bool convGPU_COMB(Conv *, GPU *);

	bool convGPU(Conv *calculated, int type, GPU *);
#endif
	void create(uint32_t row, uint32_t col);
	bool compare(Conv *);
	bool check();

	uint32_t getRow();
	uint32_t getCol();
	float* getFilter();
	uint32_t getFilterLength();
};


#endif //__conv_H_
