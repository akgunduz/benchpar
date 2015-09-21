//
//  matrix.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "matrix.h"

#define EPSILON 1.0f

Matrix::Matrix(uint32_t row, uint32_t col, GPU *gpu, Matrix *B, bool prepare)    :
		Function(gpu) {

	initFuncs();

	this->row = row;
	this->col = col;
	this->B = B;

	if (!allocMem(row, col)) {
		throw std::runtime_error("Memory insufficient!");
	}

	if (prepare) {
		create(row, col);
	}

}

Matrix::Matrix(std::string path, GPU *gpu)    :
		Function(gpu) {

	initFuncs();

	FILE *fd = fopen(path.c_str(), "r");
	if (!fd) {
		throw std::runtime_error("File could not opened!");
	}

	int res = fscanf(fd, "%d,%d", &row, &col);
	if (res == EOF) {
		fclose(fd);
		throw std::runtime_error("File Read Error happened!");
	}

	if (!allocMem(row, col)) {
		fclose(fd);
		throw std::runtime_error("Memory insufficient!");
	}

	B = new Matrix(col, row, gpu, nullptr, false);

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			res = fscanf(fd, "%f,", mem + i * col + j);
			if (res == EOF) {
				fclose(fd);
				throw std::runtime_error("File Read Error happened!");
			}
		}
	}

	for (int i = 0; i < col; i++) {

		for (int j = 0; j < row; j++) {

			res = fscanf(fd, "%f,", B->mem + i * row + j);
			if (res == EOF) {
				fclose(fd);
				throw std::runtime_error("File Read Error happened!");
			}
		}
	}

	fclose(fd);
}

Matrix::~Matrix() {

#if defined (__ARM__) && defined (__OPENCL__)
	cl_int errCode = clEnqueueUnmapMemObject(gpu->clCommandQue, buf_mem, mem, 0, NULL, NULL);
	gpu->checkErr("clEnqueueUnmapMemObject, mem", errCode);
	clReleaseMemObject(buf_mem);
#else
#ifdef __OPENCL__
	clReleaseMemObject(buf_mem);
#endif
	free(mem);
#endif

	delete[] funcList;

	if (B != nullptr) {
		delete B;
	}
}

void Matrix::initFuncs() {

	const char *kernelIDs[] = {
			"matrixMul",
			"matrixMulVec4",
			"matrixMulVec8",
			"matrixMulDiscrete"
	};

	int args[] = {
#ifdef __ARM__
		1, 1, 4, 4,
		4, 1, 4, 4,
		8, 1, 4, 4,
#else
		1, 1, 32, 8,
		4, 1, 32, 8,
		8, 1, 32, 8,
		1, 1, 16, 16
#endif
	};

	funcList = FuncList::createArray(MULTYPE_MAX, gpu);
	funcList[MULTYPE_CPU_STD].set("MULTYPE_CPU_STD", (fFuncs)&Matrix::multiplyCPU_STD, 0, 0);
	funcList[MULTYPE_CPU_TILED].set("MULTYPE_CPU_TILED", (fFuncs)&Matrix::multiplyCPU_TILED, 0, 0);
	//funcList[MULTYPE_CPU_TILED_BASIC].set("MULTYPE_CPU_TILED_BASIC", (fFuncs)&Matrix::multiplyCPU_TILED_BASIC, 0, 0);
	funcList[MULTYPE_CPU_TILED_OMP].set("MULTYPE_CPU_TILED_OMP", (fFuncs)&Matrix::multiplyCPU_TILED_OMP, 0, 0);
#ifdef __OPENCL__
	funcList[MULTYPE_GPU_STD].set("MULTYPE_GPU_STD", (fFuncs)&Matrix::multiplyGPU_STD, 1, 4, &kernelIDs[0], &args[0]);
	funcList[MULTYPE_GPU_VEC4].set("MULTYPE_GPU_VEC4", (fFuncs)&Matrix::multiplyGPU_VEC4, 1, 4, &kernelIDs[1], &args[4]);
	//funcList[MULTYPE_GPU_VEC8].set("MULTYPE_GPU_VEC8", (fFuncs)&Matrix::multiplyGPU_VEC8, 1, 4, &kernelIDs[2], &args[8]);
#ifndef __ARM__
	funcList[MULTYPE_GPU_DISC].set("MULTYPE_GPU_DISC", (fFuncs)&Matrix::multiplyGPU_DISC, 1, 4, &kernelIDs[3], &args[12]);
#endif
#endif
}

bool Matrix::allocMem(uint32_t row, uint32_t col) {

	size = (size_t) (row * col);
	mem_size = sizeof(float) * size;

#if defined (__ARM__) && defined (__OPENCL__)
	cl_int errCode;
	buf_mem = clCreateBuffer(gpu->clGPUContext, CL_MEM_READ_WRITE |
			CL_MEM_ALLOC_HOST_PTR, mem_size, NULL, &errCode);
	gpu->checkErr("clCreateBuffer", errCode);

	mem = (float *) clEnqueueMapBuffer(gpu->clCommandQue, buf_mem, CL_TRUE,
			CL_MAP_READ | CL_MAP_WRITE, 0, mem_size, 0, NULL, NULL, &errCode);
	gpu->checkErr("clEnqueueMapBuffer", errCode);
#else
	int res = posix_memalign((void**)&mem, ALIGNMENT, mem_size);
	if (res != 0) {
		printf("Alloc failed! : %d\n", errno);
		return false;
	}

	int check = (int)((unsigned long long)mem % ALIGNMENT);
	if (check != 0) {
		free(mem);
		printf("Alignment failed!\n");
		return false;
	}

#ifdef __OPENCL__
	cl_int errCode;
	buf_mem = clCreateBuffer(gpu->clGPUContext, CL_MEM_READ_WRITE,
			mem_size, NULL, &errCode);
	gpu->checkErr("clCreateBuffer", errCode);
#endif
#endif
	memset(mem, 0, mem_size);
	return true;
}

void Matrix::create(uint32_t row, uint32_t col) {

	printf("Preparing Input in Sizes; Row: %d, Column: %d\n", row, col);

	srand((unsigned)time(NULL));

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			*(mem + i * col + j) = 1.0f; //(float)(rand()  / (RAND_MAX + 1.));

		}
	}
}

bool Matrix::compare(Matrix *ref) {

	printf("Comparing Matrixes\n");

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			float val1 = *(mem + i * col + j);
			float val2 = *(ref->mem + i * col + j);
			if (EPSILON < fabsf(val1 - val2)) {
				printf("Compare failed on row : %d, col : %d\n", i, j);
				printf("First val : %f, Second val : %f\n", val1, val2);
				return false;
			}

		}
	}

	return true;
}

bool Matrix::printToFile(const char *path, uint32_t printID) {

	std::string file(path);
	file.append("/matrix/MatrixInput_" + std::to_string(printID));
	FILE *fd = fopen(file.c_str(), "w");
	if (!fd) {
		return false;
	}

	fprintf(fd, "%d,%d\n\n", row, col);

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			fprintf(fd, "%f,", *(mem + i * col + j));

		}

	}

	if (B != nullptr) {

		for (int i = 0; i < B->row; i++) {

			for (int j = 0; j < B->col; j++) {

				fprintf(fd, "%f,", *(B->mem + i * B->col + j));

			}

		}
	}

	fclose(fd);

	return true;

}

bool Matrix::check(Matrix *ref) {

	//Currently only square matrixes allowed
	if (col != ref->row || row != ref->col) {
		printf("Matrix is not suitable for multiplication\n");
		return false;
	}

	return true;
}

uint32_t Matrix::getRow() {

	return row;
}

uint32_t Matrix::getCol() {

	return col;
}
