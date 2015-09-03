//
//  matrix.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "matrix.h"

#define EPSILON 1.0f

Matrix::Matrix(uint32_t row, uint32_t col, bool prepare) {

	this->row = row;
	this->col = col;

	if (!allocMem(row, col)) {
		throw std::runtime_error("Memory insufficient!");
	}

	if (prepare) {
		create(row, col);
	}
}

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

	fclose(fd);
}

Matrix::~Matrix() {

	free(mem);
}

bool Matrix::allocMem(uint32_t row, uint32_t col) {

	size = (size_t) (row * col);
	mem_size = sizeof(float) * size;
	mem = (float*) calloc(size, sizeof(float));
	if (mem == NULL) {
		printf("Memory insufficient!\n");
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

bool Matrix::compare(Matrix *B) {

	printf("Comparing Matrixes\n");

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < col; j++) {

			float val1 = *(mem + i * col + j);
			float val2 = *(B->mem + i * col + j);
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

bool Matrix::printToFile(uint32_t printID) {

	std::string file(getcwd(NULL, 0));
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
