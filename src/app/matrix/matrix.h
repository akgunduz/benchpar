//
//  matrix.h
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#ifndef __MATRIX_H_
#define __MATRIX_H_

#include "gpu.h"

class Matrix;

typedef bool (Matrix::*fMultiplier)(Matrix *, Matrix *, GPU *);

enum MULTYPE {
	MULTYPE_CPU_STD,
	MULTYPE_CPU_TILED,
	MULTYPE_CPU_TILED_BASIC,
	MULTYPE_CPU_TILED_OMP,
	MULTYPE_GPU_STD,
	MULTYPE_GPU_VEC4,
	MULTYPE_GPU_VEC8,
	MULTYPE_MAX,
};

struct Multipliers {

	const char id[255];
	const char kernelid[255];
	int divider;
	fMultiplier f;
};

class Matrix {

	uint32_t row;
	uint32_t col;

	size_t size;
	size_t mem_size;
	float *mem;

	bool allocMem(uint32_t row, uint32_t col);

public:

	Multipliers multipliers[MULTYPE_MAX] = {
			{"MULTYPE_CPU_STD", "", 1, &Matrix::multiplyCPU_STD},
			{"MULTYPE_CPU_TILED", "", 1, &Matrix::multiplyCPU_TILED},
			{"MULTYPE_CPU_TILED_BASIC", "", 1, &Matrix::multiplyCPU_TILED_BASIC},
			{"MULTYPE_CPU_TILED_OMP", "", 1, &Matrix::multiplyCPU_TILED_OMP},
			{"MULTYPE_GPU_STD", "matrixMul", 1, &Matrix::multiplyGPU_STD},
			{"MULTYPE_GPU_VEC4", "matrixMulVec4", 1, &Matrix::multiplyGPU_VEC4},
			{"MULTYPE_GPU_VEC8", "matrixMulVec8", 1, &Matrix::multiplyGPU_VEC8},
	};

	Matrix(uint32_t row, uint32_t col, bool prepare = false);
	Matrix(std::string path);
	~Matrix();

	void printOut();
	bool printToFile(uint32_t);

	bool multiplyCPU_STD(Matrix *, Matrix *, GPU *);
	bool multiplyCPU_TILED(Matrix *, Matrix *, GPU *);
	bool multiplyCPU_TILED_BASIC(Matrix *, Matrix *, GPU *);
	bool multiplyCPU_TILED_OMP(Matrix *, Matrix *, GPU *);
	bool multiplyGPU_STD(Matrix *, Matrix *, GPU *);
	bool multiplyGPU_VEC4(Matrix *, Matrix *, GPU *);
	bool multiplyGPU_VEC8(Matrix *, Matrix *, GPU *);

	bool multiplyGPU(Matrix *ref, Matrix *, enum MULTYPE mulType, GPU *);

	void create(uint32_t row, uint32_t col);
	bool compare(Matrix *);
	bool check(Matrix *ref);

	uint32_t getRow();
	uint32_t getCol();
};

#endif
