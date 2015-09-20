//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//

#include "scan.h"

#define EPSILON 10.0

Scan::Scan(size_t size, GPU *gpu, bool prepare)   :
		Function(gpu) {

	initFuncs();

	this->size = size;

	if (!allocMem(size)) {
		throw std::runtime_error("Memory insufficient!");
	}

	if (prepare) {
		create(size);
	}
}

Scan::Scan(std::string path, GPU *gpu)  :
		Function(gpu) {

	initFuncs();

	FILE *fd = fopen(path.c_str(), "r");
	if (!fd) {
		throw std::runtime_error("File could not opened!");
	}

	int res = fscanf(fd, "%d", (int *)&size);
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
}

Scan::~Scan() {

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
}

void Scan::initFuncs() {

	const char *kernelIDs[] = {
			"scanExclusiveLocal1",
			"scanExclusiveLocal2",
			"uniformUpdate"
	};

	funcList = FuncList::createArray(SCANTYPE_MAX, gpu);
	funcList[SCANTYPE_CPU_STD].set("SCANTYPE_CPU_STD", (fFuncs)&Scan::scanCPU_STD, 0, 0);
#ifndef __ARM__
	funcList[SCANTYPE_CPU_AVX].set("SCANTYPE_CPU_AVX", (fFuncs)&Scan::scanCPU_AVX, 0, 0);
	funcList[SCANTYPE_CPU_SSE].set("SCANTYPE_CPU_SSE", (fFuncs)&Scan::scanCPU_SSE, 0, 0);
	funcList[SCANTYPE_CPU_OMP_SSE].set("SCANTYPE_CPU_OMP_SSE", (fFuncs)&Scan::scanCPU_OMP_SSE, 0, 0);
	funcList[SCANTYPE_CPU_OMP_SSEp2_SSEp1].set("SCANTYPE_CPU_OMP_SSEp2_SSEp1", (fFuncs)&Scan::scanCPU_OMP_SSEp2_SSEp1, 0, 0);
#endif
#ifdef __OPENCL__
	funcList[SCANTYPE_GPU_STD].set("SCANTYPE_GPU_STD", (fFuncs)&Scan::scanGPU_STD, 3, 0, &kernelIDs[0]);
#endif
}

bool Scan::allocMem(size_t size) {

	this->size = size;
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

void Scan::create(size_t size) {

	printf("Preparing Input in Size: %ld\n", size);

	srand((unsigned)time(NULL));

	for (int i = 0; i < size; i++) {

		mem[i] = (float)(rand()  / (RAND_MAX + 1.));

	}
}

bool Scan::compare(Scan *ref) {

	printf("Comparing Matrixes\n");

	for (int i = 0; i < size - 1; i++) {

		float val1 = mem[i];
		float val2 = ref->mem[i + 1];
		if (EPSILON < fabsf(val1 - val2)) {
			printf("Compare failed on index : %d\n", i);
			printf("First val : %f, Second val : %f\n", val1, val2);
			return false;
		}
	}

	return true;
}

bool Scan::printToFile(const char *path, uint32_t printID) {

	std::string file(path);
	file.append("/scan/ScanInput_" + std::to_string(printID));
	FILE *fd = fopen(file.c_str(), "w");
	if (!fd) {
		return false;
	}

	fprintf(fd, "%d\n", (int)size);

	for (int i = 0; i < size; i++) {

		fprintf(fd, "%f,", mem[i]);
	}

	fclose(fd);

	return true;

}

bool Scan::check() {

	return true;
}

size_t Scan::getSize() {

	return size;
}
