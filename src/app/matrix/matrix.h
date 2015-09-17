//
//  matrix.h
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#ifndef __MATRIX_H_
#define __MATRIX_H_

#include "function.h"

enum MULTYPE {
	MULTYPE_CPU_STD,
	MULTYPE_CPU_TILED,
	MULTYPE_CPU_TILED_BASIC,
	MULTYPE_CPU_TILED_OMP,
        MULTYPE_CPU_MAX,
#ifdef __OPENCL__
	MULTYPE_GPU_STD = MULTYPE_CPU_MAX,
	MULTYPE_GPU_VEC4,
	MULTYPE_GPU_VEC8,
        MULTYPE_MAX
#else
        MULTYPE_MAX = MULTYPE_CPU_MAX,
#endif
};


class Matrix : public Function {

	uint32_t row;
	uint32_t col;

	bool allocMem(uint32_t row, uint32_t col);

public:

	Matrix *B;

	Matrix(uint32_t row, uint32_t col, Matrix *B = nullptr, bool prepare = false);
	Matrix(std::string path);
	~Matrix();

	virtual void initFuncs();

	void printOut();
	bool printToFile(const char *, uint32_t);

	bool multiplyCPU_STD(Matrix *, GPU *);
	bool multiplyCPU_TILED(Matrix *, GPU *);
	bool multiplyCPU_TILED_BASIC(Matrix *, GPU *);
	bool multiplyCPU_TILED_OMP(Matrix *, GPU *);
#ifdef __OPENCL__
	bool multiplyGPU_STD(Matrix *, GPU *);
	bool multiplyGPU_VEC4(Matrix *, GPU *);
	bool multiplyGPU_VEC8(Matrix *, GPU *);

	bool multiplyGPU(Matrix *ref, int type, GPU *);
#endif
	void create(uint32_t row, uint32_t col);
	bool compare(Matrix *);
	bool check(Matrix *ref);

	uint32_t getRow();
	uint32_t getCol();
};

#endif
