//
//  multiplications.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "matrix.h"

bool Matrix::multiplyCPU_STD(Matrix *ref, Matrix *calculated, GPU *gpu) {

	for (int i = 0; i < row; i++) {

		for (int j = 0; j < ref->col; j++) {

			for (int k = 0; k < col; k++) {

				calculated->mem[i * ref->col + j] +=
						mem[i * col + k] * ref->mem[k * ref->col + j];
			}
		}
	}

	return true;
}

bool Matrix::multiplyCPU_TILED(Matrix *ref, Matrix *calculated, GPU *gpu) {

	for (int i = 0; i < ref->col; i += TILESIZE) {

		for (int k = 0; k < col; k += TILESIZE) {

			for (int j = 0; j < row; j++) {

				for (int kk = k; kk < k + TILESIZE; kk++) {

					for (int ii = i; ii < i + TILESIZE; ii++) {

						calculated->mem[j * ref->col + ii] +=
								mem[j * col + kk] * ref->mem[kk * ref->col + ii];
					}
				}
			}
		}
	}

	return true;
}

bool Matrix::multiplyCPU_TILED_BASIC(Matrix *ref, Matrix *calculated, GPU *gpu) {

	for (int j0 = 0; j0 < row; j0 += TILESIZE) {

		for (int i0 = 0; i0 < ref->col; i0 += TILESIZE) {

			for (int k0 = 0; k0 < col; k0 += TILESIZE) {

				for (int j = j0; j < j0 + TILESIZE; j++) {

					for (int i = i0; i < i0 + TILESIZE; i++) {

						for (int k = k0; k < k0 + TILESIZE; k++) {

							calculated->mem[j * ref->col + i] +=
									mem[j * col + k] * ref->mem[k * ref->col + i];
						}
					}
				}
			}
		}
	}

	return true;
}

bool Matrix::multiplyCPU_TILED_OMP(Matrix *ref, Matrix *calculated, GPU *gpu) {

#pragma omp parallel for
	for (int i = 0; i < ref->col; i += TILESIZE) {
#pragma omp parallel for
		for (int k = 0; k < col; k += TILESIZE) {
#pragma omp parallel for
			for (int j = 0; j < row; j++) {

				for (int kk = k; kk < k + TILESIZE; kk++) {

					for (int ii = i; ii < i + TILESIZE; ii++) {

						calculated->mem[j * ref->col + ii] +=
								mem[j * col + kk] * ref->mem[kk * ref->col + ii];
					}
				}
			}
		}
	}

	return true;
}

bool Matrix::multiplyGPU_STD(Matrix *ref, Matrix *calculated, GPU *gpu) {

	return multiplyGPU(ref, calculated, MULTYPE_GPU_STD, gpu);
}

bool Matrix::multiplyGPU_VEC4(Matrix *ref, Matrix *calculated, GPU *gpu) {

	return multiplyGPU(ref, calculated, MULTYPE_GPU_VEC4, gpu);
}

bool Matrix::multiplyGPU_VEC8(Matrix *ref, Matrix *calculated, GPU *gpu) {

	return multiplyGPU(ref, calculated, MULTYPE_GPU_VEC8, gpu);
}

bool Matrix::multiplyGPU(Matrix *ref, Matrix *calculated, enum MULTYPE mulType, GPU *gpu) {

	if (!gpu->getEnabled()) {
		return false;
	}

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

	return true;
}
