//
//  scanners.cpp
//  ScanCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "scan.h"

bool Scan::scanCPU_STD(Scan *calculated, GPU *gpu) {

	float sum = 0;
	for (int i = 0; i < size; i++) {
		sum += mem[i];
		calculated->mem[i] = sum;
	}

	return true;
}

#ifndef C15

inline __m256 scan_AVX(__m256 x)
{
	__m256 t0, t1;
	//shift1_AVX + add
	t0 = _mm256_permute_ps(x, _MM_SHUFFLE(2, 1, 0, 3));
	t1 = _mm256_permute2f128_ps(t0, t0, 41);
	x = _mm256_add_ps(x, _mm256_blend_ps(t0, t1, 0x11));
	//shift2_AVX + add
	t0 = _mm256_permute_ps(x, _MM_SHUFFLE(1, 0, 3, 2));
	t1 = _mm256_permute2f128_ps(t0, t0, 41);
	x = _mm256_add_ps(x, _mm256_blend_ps(t0, t1, 0x33));
	//shift3_AVX + add
	x = _mm256_add_ps(x,_mm256_permute2f128_ps(x, x, 41));
	return x;
}

bool Scan::scanCPU_AVX(Scan *calculated, GPU *gpu) {

	__m256 offset = _mm256_setzero_ps();
	for (int i = 0; i < size; i += 8)
	{
		__m256 x = _mm256_loadu_ps(&mem[i]);
		__m256 out = scan_AVX(x);
		out = _mm256_add_ps(out, offset);
		_mm256_storeu_ps(&calculated->mem[i], out);
		//broadcast last element
		__m256 t0 = _mm256_permute2f128_ps(out, out, 0x11);
		offset = _mm256_permute_ps(t0, 0xff);
	}

	return true;
}

inline __m128 scan_SSE(__m128 x)
{
	x = _mm_add_ps(x, _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(x), 4)));
	x = _mm_add_ps(x, _mm_castsi128_ps(_mm_slli_si128(_mm_castps_si128(x), 8)));
	return x;
}

bool Scan::scanCPU_SSE(Scan *calculated, GPU *gpu) {

	__m128 offset = _mm_setzero_ps();
	for (int i = 0; i < size; i+=4)
	{
		__m128 x = _mm_load_ps(&mem[i]);
		__m128 out = scan_SSE(x);
		out = _mm_add_ps(out, offset);
		_mm_store_ps(&calculated->mem[i], out);
		offset = _mm_shuffle_ps(out, out, _MM_SHUFFLE(3, 3, 3, 3));
	}

	return true;
}

bool Scan::scanCPU_OMP_SSE(Scan *calculated, GPU *gpu) {

	float *suma;
#pragma omp parallel
	{
		const int ithread = omp_get_thread_num();
		const int nthreads = omp_get_num_threads();
#pragma omp single
		{
			suma = new float[nthreads + 1];
			suma[0] = 0;
		}
		float sum = 0;
#pragma omp for schedule(static) nowait //first parallel pass
		for (int i = 0; i < size; i++) {
			sum += mem[i];
			calculated->mem[i] = sum;
		}
		suma[ithread + 1] = sum;
#pragma omp barrier
#pragma omp single
		{
			float tmp = 0;
			for (int i = 0; i<(nthreads + 1); i++) {
				tmp += suma[i];
				suma[i] = tmp;
			}
		}
		__m128 offset = _mm_set1_ps(suma[ithread]);
#pragma omp for schedule(static) //second parallel pass with SSE as well
		for (int i = 0; i < size / 4; i++) {
			__m128 tmp = _mm_load_ps(&calculated->mem[4*i]);
			tmp = _mm_add_ps(tmp, offset);
			_mm_store_ps(&calculated->mem[4*i], tmp);
		}
	}
	delete[] suma;

	return true;
}

bool Scan::scanCPU_OMP_SSEp2_SSEp1(Scan *calculated, GPU *gpu) {

	__m128 offset = _mm_setzero_ps();
	for (int i = 0; i < size; i+=4)
	{
		__m128 x = _mm_load_ps(&mem[i]);
		__m128 out = scan_SSE(x);
		out = _mm_add_ps(out, offset);
		_mm_store_ps(&calculated->mem[i], out);
		offset = _mm_shuffle_ps(out, out, _MM_SHUFFLE(3, 3, 3, 3));
	}

	return true;
}

#endif


bool Scan::scanGPU_STD(Scan *calculated, GPU *gpu) {

	return scanGPU(calculated, SCANTYPE_GPU_STD, gpu);
}

bool Scan::scanGPU_VEC4(Scan *calculated, GPU *gpu) {

	return scanGPU(calculated, SCANTYPE_GPU_VEC4, gpu);
}

bool Scan::scanGPU_VEC8(Scan *calculated, GPU *gpu) {

	return scanGPU(calculated, SCANTYPE_GPU_VEC8, gpu);
}

bool Scan::scanGPU(Scan *calculated, enum SCANTYPE scanType, GPU *gpu) {

	if (!gpu->getEnabled()) {
		return false;
	}
/*
	cl_int errCode;

	cl_mem d_C = clCreateBuffer(gpu->clGPUContext,
			CL_MEM_READ_WRITE,
			(size_t) (calculated->mem_size),
			NULL,
			&errCode);

	cl_mem d_A = clCreateBuffer(gpu->clGPUContext,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			(size_t) (mem_size),
			mem,
			&errCode);

	cl_mem d_B = clCreateBuffer(gpu->clGPUContext,
			CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
			(size_t) (ref->mem_size),
			ref->mem,
			&errCode);

	cl_kernel clKernel = clCreateKernel(gpu->clProgram,
			multipliers[mulType].kernelid, &errCode);
	gpu->checkErr("clCreateKernel", errCode);

	gpu->globalWorkSize[0] = ref->col;
	gpu->globalWorkSize[1] = row;

	int colVec = col / multipliers[mulType].divider;

	errCode = clSetKernelArg(clKernel, 0, sizeof(cl_mem), (void *) &d_A);
	errCode |= clSetKernelArg(clKernel, 1, sizeof(cl_mem), (void *) &d_B);
	errCode |= clSetKernelArg(clKernel, 2, sizeof(cl_mem), (void *) &d_C);
	errCode |= clSetKernelArg(clKernel, 3, sizeof(int), (void *) &colVec);
	errCode |= clSetKernelArg(clKernel, 4, sizeof(int), (void *) &ref->col);

	errCode = clEnqueueNDRangeKernel(gpu->clCommandQue,
			clKernel,
			2,
			NULL,
			gpu->globalWorkSize,
			NULL, //localWorkSize,
			0,
			NULL,
			NULL);
	gpu->checkErr("clEnqueueNDRangeKernel", errCode);

	//retrieve result from device
	errCode = clEnqueueReadBuffer(gpu->clCommandQue,
			d_C,
			CL_TRUE,
			0,
			calculated->mem_size,
			calculated->mem,
			0,
			NULL,
			NULL);
	gpu->checkErr("clEnqueueReadBuffer", errCode);

	clFinish(gpu->clCommandQue);

	clReleaseMemObject(d_A);
	clReleaseMemObject(d_C);
	clReleaseMemObject(d_B);

	clReleaseKernel(clKernel);
*/
	return true;
}
