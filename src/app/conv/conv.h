//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __conv_H_
#define __conv_H_

#include "gpu.h"

#define HEIGHT 1024
#define WIDTH 1024
#define FILTER_LENGTH 5
#define RANGE 10
#define ITERATIONS 100

#define KERNEL_RADIUS 2
#define KERNEL_LENGTH (2 * KERNEL_RADIUS + 1)

#define ROWS_BLOCKDIM_X 16//32
#define ROWS_BLOCKDIM_Y 4//8
#define COLUMNS_BLOCKDIM_X 16//32
#define COLUMNS_BLOCKDIM_Y 8
#define ROWS_RESULT_STEPS 8//4
#define ROWS_HALO_STEPS 1
#define COLUMNS_RESULT_STEPS 8//4
#define COLUMNS_HALO_STEPS 1

class Conv;

typedef bool (Conv::*fConvFuncs)(Conv *, GPU *);

enum CONVTYPE {

	CONVTYPE_CPU_STD,
	CONVTYPE_CPU_OMP,

	CONVTYPE_GPU_STD,

	CONVTYPE_MAX,
};

struct ConvFuncs {

	const char id[255];
	const char kernelid[255];
	int divider;
	fConvFuncs f;
};

class Conv {

	uint32_t row;
	uint32_t col;

	size_t size;
	float *mem;
	size_t mem_size;
	float *temp;
	float *filter;
	uint32_t filter_length;

	bool allocMem(uint32_t row, uint32_t col);

public:

	ConvFuncs convFuncs[CONVTYPE_MAX] = {
			{"CONVTYPE_CPU_STD", "", 1, &Conv::convCPU_STD},
			{"CONVTYPE_CPU_OMP", "", 1, &Conv::convCPU_OMP},
			{"SCANTYPE_GPU_STD", "matrixMul", 1, &Conv::convGPU_STD},
	};

	Conv(uint32_t row, uint32_t col, float *filter, uint32_t filter_length, bool prepare = false);
	Conv(std::string path, float *filter, uint32_t filter_length);
	~Conv();

	void printOut();
	bool printToFile(uint32_t);

	bool convCPU_STD(Conv *, GPU *);
	bool convCPU_OMP(Conv *, GPU *);

	bool convGPU_STD(Conv *, GPU *);

	bool convGPU(Conv *calculated, enum CONVTYPE convType, GPU *);

	void create(uint32_t row, uint32_t col);
	bool compare(Conv *);
	bool check();

	uint32_t getRow();
	uint32_t getCol();
	float* getFilter();
	uint32_t getFilterLength();
};


#endif //__conv_H_
