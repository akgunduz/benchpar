//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//

#include "conv.h"

#define EPSILON 0.001f //FLT_MIN

Conv::Conv(uint32_t row, uint32_t col, float *filter, uint32_t filter_length, bool prepare) {

	this->row = row;
	this->col = col;
	this->filter_length = filter_length;
	this->filter = filter;


	if (!allocMem(row, col)) {
		throw std::runtime_error("Memory insufficient!");
	}

	if (prepare) {
		create(row, col);
	}

	initFuncs();
}

Conv::Conv(std::string path, float *filter, uint32_t filter_length) {

	FILE *fd = fopen(path.c_str(), "r");
	if (!fd) {
		throw std::runtime_error("File could not opened!");
	}

	int res = fscanf(fd, "%d,%d", &row, &col);
	if (res == EOF) {
		fclose(fd);
		throw std::runtime_error("File Read Error happened!");
	}

	this->filter_length = filter_length;
	this->filter = filter;

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

	fclose(fd);

	initFuncs();
}

Conv::~Conv() {

	free(mem);

	delete[] funcList;
}

void Conv::initFuncs() {

	funcList = new FuncList[CONVTYPE_MAX];
        const char *kernelIDs[] = {
                "convRows5_float",
                "convRows5_float",
                
                "convRows5Vec4_float",
                "convCols5Vec4_float",
        };
        
	funcList[CONVTYPE_CPU_STD].set("CONVTYPE_CPU_STD", (fFuncs)&Conv::convCPU_STD, 0);
	funcList[CONVTYPE_CPU_OMP].set("CONVTYPE_CPU_OMP", (fFuncs)&Conv::convCPU_OMP, 0);
#ifdef __OPENCL__
	funcList[CONVTYPE_GPU_STD].set("CONVTYPE_GPU_STD", (fFuncs)&Conv::convGPU_STD, 2, &kernelIDs[0]);
        funcList[CONVTYPE_GPU_VEC4].set("CONVTYPE_GPU_VEC4", (fFuncs)&Conv::convGPU_VEC4, 2, &kernelIDs[/*2*/0]);
#endif
}

bool Conv::allocMem(uint32_t row, uint32_t col) {

	size = (size_t) (row * col);
	mem_size = sizeof(float) * size;

	int res = posix_memalign((void**)&mem, ALIGNMENT, mem_size);
	if (res != 0) {
		printf("Alloc failed! : %d\n", errno);
		return false;
	}

	res = posix_memalign((void**)&temp, ALIGNMENT, mem_size);
	if (res != 0) {
		printf("Alloc failed! : %d\n", errno);
		free(mem);
		return false;
	}

	int check = (int)((unsigned long long)mem % ALIGNMENT);
	check |= (int)((unsigned long long)temp % ALIGNMENT);
	if (check != 0) {
		free(mem);
		free(temp);
		printf("Alignment failed!\n");
		return false;
	}

	return true;

}

void Conv::create(uint32_t row, uint32_t col) {

	printf("Preparing Input in Sizes; Row: %d, Column: %d\n", row, col);

	srand((unsigned)time(NULL));

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			*(mem + i * col + j) = (float)(rand()  / (RAND_MAX + 1.));

		}
	}
}

bool Conv::compare(Conv *ref) {

	printf("Comparing Convs\n");

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

void Conv::printOut() {

	printf("Printing Out Conv in Sizes; Row: %d, Column: %d\n", row, col);

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

bool Conv::printToFile(const char *path, uint32_t printID) {

	std::string file(path);
	file.append("/conv/ConvInput_" + std::to_string(printID));
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

bool Conv::check() {

	return true;
}

uint32_t Conv::getRow() {

	return row;
}

uint32_t Conv::getCol() {

	return col;
}

float* Conv::getFilter() {

	return filter;
}

uint32_t Conv::getFilterLength() {

	return filter_length;
}
