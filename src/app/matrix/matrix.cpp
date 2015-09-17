//
//  matrix.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "matrix.h"

#define EPSILON 1.0f

Matrix::Matrix(uint32_t row, uint32_t col, Matrix *B, bool prepare) {

	this->row = row;
	this->col = col;
	this->B = B;

	if (!allocMem(row, col)) {
		throw std::runtime_error("Memory insufficient!");
	}

	if (prepare) {
		create(row, col);
	}

	initFuncs();
}

#if 0
Matrix::Matrix(std::string path) {

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

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			res = fscanf(fd, "%f,", mem + i * col + j);
			if (res == EOF) {
				fclose(fd);
				throw std::runtime_error("File Read Error happened!");
			}
		}
	}

	this->B = nullptr;

	fclose(fd);
}
#endif


Matrix::Matrix(std::string path) {

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

	B = new Matrix(col, row, nullptr, false);

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

	initFuncs();
}

Matrix::~Matrix() {

	free(mem);
	if (B != nullptr) {
		delete B;
	}

	delete[] funcList;
}

void Matrix::initFuncs() {

        const char *kernelIDs[] = {
                "matrixMul",
                "matrixMulVec4",
                "matrixMulVec8"
        };
        
	funcList = new FuncList[MULTYPE_MAX];
	funcList[MULTYPE_CPU_STD].set("MULTYPE_CPU_STD", (fFuncs)&Matrix::multiplyCPU_STD, 0);
	funcList[MULTYPE_CPU_TILED].set("MULTYPE_CPU_TILED", (fFuncs)&Matrix::multiplyCPU_TILED, 0);
	funcList[MULTYPE_CPU_TILED_BASIC].set("MULTYPE_CPU_TILED_BASIC", (fFuncs)&Matrix::multiplyCPU_TILED_BASIC, 0);
	funcList[MULTYPE_CPU_TILED_OMP].set("MULTYPE_CPU_TILED_OMP", (fFuncs)&Matrix::multiplyCPU_TILED_OMP, 0);
#ifdef __OPENCL__
	funcList[MULTYPE_GPU_STD].set("MULTYPE_GPU_STD", (fFuncs)&Matrix::multiplyGPU_STD, 1, &kernelIDs[0]);
	funcList[MULTYPE_GPU_VEC4].set("MULTYPE_GPU_VEC4", (fFuncs)&Matrix::multiplyGPU_VEC4, 1, &kernelIDs[1]);
	funcList[MULTYPE_GPU_VEC8].set("MULTYPE_GPU_VEC8", (fFuncs)&Matrix::multiplyGPU_VEC8, 1, &kernelIDs[2]);
#endif
}

bool Matrix::allocMem(uint32_t row, uint32_t col) {

	size = (size_t) (row * col);
	mem_size = sizeof(float) * size;

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

	return true;
}

void Matrix::create(uint32_t row, uint32_t col) {

	printf("Preparing Input in Sizes; Row: %d, Column: %d\n", row, col);

	srand((unsigned)time(NULL));

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			*(mem + i * col + j) = (float)(rand()  / (RAND_MAX + 1.));

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

void Matrix::printOut() {

	printf("Printing Out Matrix in Sizes; Row: %d, Column: %d\n", row, col);

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			printf("%f", *(mem + i * col + j));
			if (j < col - 1) {
				printf(",");
			}

		}
		printf("\n");
	}

}

#if 0
bool Matrix::printToFile(const char *path, uint32_t printID) {

	std::string file(path);
	file.append("/MatrixInput_" + std::to_string(printID));
	FILE *fd = fopen(file.c_str(), "w");
	if (!fd) {
		return false;
	}

	fprintf(fd, "%d,%d\n\n", row, col);

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			fprintf(fd, "%f", *(mem + i * col + j));
			if (j < col - 1) {
				fprintf(fd, ",");
			}

		}
		fprintf(fd, "\n");
	}

	fclose(fd);

	return true;

}
#endif

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
