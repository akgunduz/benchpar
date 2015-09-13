//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//

#include "scan.h"

#define EPSILON 100.0

Scan::Scan(uint32_t size, bool prepare) {

	this->size = size;

	if (!allocMem(size)) {
		throw std::runtime_error("Memory insufficient!");
	}

	if (prepare) {
		create(size);
	}

	initFuncs();
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

		res = fscanf(fd, "%f,", &mem[i]);
		if (res == EOF) {
			fclose(fd);
			throw std::runtime_error("File Read Error happened!");
		}
	}

	fclose(fd);

	initFuncs();
}

Scan::~Scan() {

	free(mem);

	delete[] funcList;
}

void Scan::initFuncs() {

	funcList = new FuncList[SCANTYPE_MAX];
	funcList[SCANTYPE_CPU_STD].set("SCANTYPE_CPU_STD", "", 1, (fFuncs)&Scan::scanCPU_STD);
#ifndef __ARM__
	funcList[SCANTYPE_CPU_AVX].set("SCANTYPE_CPU_AVX", "", 1, (fFuncs)&Scan::scanCPU_AVX);
	funcList[SCANTYPE_CPU_SSE].set("SCANTYPE_CPU_SSE", "", 1, (fFuncs)&Scan::scanCPU_SSE);
	funcList[SCANTYPE_CPU_OMP_SSE].set("SCANTYPE_CPU_OMP_SSE", "", 1, (fFuncs)&Scan::scanCPU_OMP_SSE);
	funcList[SCANTYPE_CPU_OMP_SSEp2_SSEp1].set("SCANTYPE_CPU_OMP_SSEp2_SSEp1", "", 1, (fFuncs)&Scan::scanCPU_OMP_SSEp2_SSEp1);
#endif
	funcList[SCANTYPE_GPU_STD].set("SCANTYPE_GPU_STD", "", 1, (fFuncs)&Scan::scanGPU_STD);
}

bool Scan::allocMem(uint32_t size) {

	this->size = size;
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

void Scan::create(uint32_t size) {

	printf("Preparing Input in Size: %d\n", size);

	srand((unsigned)time(NULL));

	for (int i = 0; i < size; i++) {

		mem[i] = (float)(rand()  / (RAND_MAX + 1.));

	}
}

bool Scan::compare(Scan *ref) {

	printf("Comparing Matrixes\n");

	for (int i = 0; i < size; i++) {

		float val1 = mem[i];
		float val2 = ref->mem[i];
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

		printf("%f,", mem[i]);
		if (i % 30 == 0){
			printf("\n");
		}
	}
}

bool Scan::printToFile(uint32_t printID) {

	std::string file(getcwd(NULL, 0));
	file.append("/scan/ScanInput_" + std::to_string(printID));
	FILE *fd = fopen(file.c_str(), "w");
	if (!fd) {
		return false;
	}

	fprintf(fd, "%d\n", size);

	for (int i = 0; i < size; i++) {

		fprintf(fd, "%f,", mem[i]);
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
