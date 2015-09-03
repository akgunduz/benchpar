//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//

#include "scan.h"

#define SIZE 1024*512
#define ALIGNMENT 16
#define RANGE 10
#define EPSILON 100.0 //FLT_MIN
#define ITERATIONS 1000

Scan::Scan(uint32_t size, bool prepare) {

	this->size = size;

	if (!allocMem(size)) {
		throw std::runtime_error("Memory insufficient!");
	}

	if (prepare) {
		create(size);
	}
}

Scan::Scan(std::string path) {

	FILE *fd = fopen(path.c_str(), "r");
	if (!fd) {
		throw std::runtime_error("File could not opened!");
	}

	int res = fscanf(fd, "%d", &size);
	if (res == EOF) {
		fclose(fd);
		throw std::runtime_error("File Read Error happened!");
	}

	if (!allocMem(size)) {
		fclose(fd);
		throw std::runtime_error("Memory insufficient!");
	}

	for (int i = 0; i < size; i++) {

		res = fscanf(fd, "%f,", mem + i);
		if (res == EOF) {
			fclose(fd);
			throw std::runtime_error("File Read Error happened!");
		}
	}

	fclose(fd);
}

Scan::~Scan() {

	delete mem;
}

bool Scan::allocMem(uint32_t size) {

	this->size = size;
	mem_size = sizeof(float) * size;
	mem = new float[size]; //new is already aligned allocation
	if (mem == NULL) {
		printf("Memory insufficient!\n");
		return false;
	}

	int check = (int)((unsigned long long)mem % ALIGNMENT);
	if (check != 0) {
		delete mem;
		printf("Alignment failed!\n");
		return false;
	}

	return true;
}

void Scan::create(uint32_t size) {

	printf("Preparing Input in Size: %d\n", size);

	srand((unsigned)time(NULL));

	for (int i = 0; i < size; i++) {

		*(mem + i) = (float)(rand()  / (RAND_MAX + 1.));

	}
}

bool Scan::compare(Scan *ref) {

	printf("Comparing Matrixes\n");

	for (int i = 0; i < size; i++) {

		float val1 = *(mem + i);
		float val2 = *(ref->mem + i);
		if (EPSILON < fabsf(val1 - val2)) {
			printf("Compare failed on index : %d\n", i);
			printf("First val : %f, Second val : %f\n", val1, val2);
			return false;
		}
	}

	return true;
}

void Scan::printOut() {

	printf("Printing Out Scan out in Size: %d\n", size);

	for (int i = 0; i < size; i++) {

		printf("%f,", *(mem + i));
		if (i % 30 == 0){
			printf("\n");
		}
	}
}

bool Scan::printToFile(uint32_t printID) {

	std::string file(getcwd(NULL, 0));
	file.append("/ScanInput_" + std::to_string(printID));
	FILE *fd = fopen(file.c_str(), "w");
	if (!fd) {
		return false;
	}

	fprintf(fd, "%d\n\n", size);

	for (int i = 0; i < size; i++) {

		fprintf(fd, "%f,", *(mem + i));
		if (i % 30 == 0) {
			fprintf(fd, "\n");
		}
	}

	fclose(fd);

	return true;

}

bool Scan::check() {

	return true;
}

uint32_t Scan::getSize() {

	return size;
}
