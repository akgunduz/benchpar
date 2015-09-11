//
//  scanners.cpp
//  ScanCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "conv.h"

bool Conv::convCPU_STD(Conv *calculated, GPU *gpu) {

	int half_length = (filter_length - 1) / 2;

	//horizontal
	for(int j = 0; j < row; j++) {

		for(int i=(half_length+1); i<(col-(half_length+1)); i++)
		{
			float acc = 0.0f;
			for(int k=0; k<filter_length; k++)
			{
				acc += mem[j*col + (i+k-half_length)] * filter[k];
			}
			temp[j*col + i] = acc;
		}
	}

	//vertical
	for(int j=(half_length+1); j<(row-(half_length+1)); j++)
	{
		for(int i=0; i<col; i++)
		{
			float acc = 0.0f;
			for(int k=0; k<filter_length; k++)
			{
				acc += temp[(j+k-half_length)*col + i] * filter[k];
			}
			calculated->mem[j*col + i] = acc;
		}
	}

	return true;
}

bool Conv::convCPU_OMP(Conv *calculated, GPU *gpu) {

	int half_length = (filter_length - 1) / 2;

	float *local_mem = mem;
	float *local_calcmem = calculated->mem;
	float *local_temp = temp;
	float *local_filter = filter;

	//horizontal
#pragma omp parallel for shared(local_mem, local_temp) firstprivate(local_filter) schedule(static)
	for(int j=0; j<row; j++)
	{
		for(int i=(half_length+1); i<(col-(half_length+1)); i++)
		{
			float acc = 0.0f;
			for(int k=0; k<filter_length; k++)
			{
				acc += local_mem[j*col + (i+k-half_length)] * local_filter[k];
			}
			local_temp[j*col + i] = acc;
		}
	}

	//vertical
#pragma omp parallel for shared(local_calcmem, local_temp) firstprivate(local_filter) schedule(static)
	for(int j=(half_length+1); j<(row-(half_length+1)); j++)
	{
		for(int i=0; i<col; i++)
		{
			float acc = 0.0f;
			for(int k=0; k<filter_length; k++)
			{
				acc += local_temp[(j+k-half_length)*col + i] * local_filter[k];
			}
			local_calcmem[j*col + i] = acc;
		}
	}

	return true;
}


bool Conv::convGPU_STD(Conv *calculated, GPU *gpu) {

	return convGPU(calculated, CONVTYPE_GPU_STD, gpu);
}

bool Conv::convGPU(Conv *calculated, enum CONVTYPE convType, GPU *gpu) {

	if (!gpu->getEnabled()) {
		return false;
	}

	cl_int errCode;

	cl_mem buf_input = clCreateBuffer(gpu->clGPUContext,
			CL_MEM_READ_ONLY,
			(size_t) (mem_size),
			NULL,
			&errCode);

	cl_mem buf_output = clCreateBuffer(gpu->clGPUContext,
			CL_MEM_WRITE_ONLY,
			(size_t) (mem_size),
			NULL,
			&errCode);

	cl_mem buf_temp = clCreateBuffer(gpu->clGPUContext,
			CL_MEM_READ_WRITE,
			(size_t) (mem_size),
			NULL,
			&errCode);

	cl_mem buf_filter = clCreateBuffer(gpu->clGPUContext,
			CL_MEM_READ_ONLY,
			(size_t) (filter_length * sizeof(float)),
			NULL,
			&errCode);

	size_t  localWorkSizeRows[2], globalWorkSizeRows[2];
	size_t  localWorkSizeCols[2], globalWorkSizeCols[2];

	errCode = clEnqueueWriteBuffer(gpu->clCommandQue, buf_input, CL_FALSE, 0, mem_size, mem, 0, NULL, NULL);
	gpu->checkErr("clEnqueueWriteBuffer", errCode);

	errCode = clEnqueueWriteBuffer(gpu->clCommandQue, buf_filter, CL_FALSE, 0, filter_length * sizeof(float), filter, 0, NULL, NULL);
	gpu->checkErr("clEnqueueWriteBuffer", errCode);

	cl_kernel kernelConvRows = clCreateKernel(gpu->clProgram, "convolutionRows", &errCode);
	gpu->checkErr("clCreateKernel", errCode);

	cl_kernel kernelConvCols = clCreateKernel(gpu->clProgram, "convolutionColumns", &errCode);
	gpu->checkErr("clCreateKernel", errCode);


	errCode  = clSetKernelArg(kernelConvRows, 0, sizeof(cl_mem), (void*)&buf_temp);
	errCode |= clSetKernelArg(kernelConvRows, 1, sizeof(cl_mem), (void*)&buf_input);
	errCode |= clSetKernelArg(kernelConvRows, 2, sizeof(cl_mem), (void*)&buf_filter);
	errCode |= clSetKernelArg(kernelConvRows, 3, sizeof(int),    (void*)&col);
	errCode |= clSetKernelArg(kernelConvRows, 4, sizeof(int),    (void*)&row);
	errCode |= clSetKernelArg(kernelConvRows, 5, sizeof(int),    (void*)&col);
	gpu->checkErr("clSetKernelArg", errCode);

	localWorkSizeRows[0] = ROWS_BLOCKDIM_X;
	localWorkSizeRows[1] = ROWS_BLOCKDIM_Y;

	globalWorkSizeRows[0] = col / ROWS_RESULT_STEPS;
	globalWorkSizeRows[1] = row;

	errCode  = clSetKernelArg(kernelConvCols, 0, sizeof(cl_mem), (void*)&buf_output);
	errCode |= clSetKernelArg(kernelConvCols, 1, sizeof(cl_mem), (void*)&buf_temp);
	errCode |= clSetKernelArg(kernelConvCols, 2, sizeof(cl_mem), (void*)&buf_filter);
	errCode |= clSetKernelArg(kernelConvCols, 3, sizeof(int),    (void*)&col);
	errCode |= clSetKernelArg(kernelConvCols, 4, sizeof(int),    (void*)&row);
	errCode |= clSetKernelArg(kernelConvCols, 5, sizeof(int),    (void*)&col);
	gpu->checkErr("clSetKernelArg", errCode);


	localWorkSizeCols[0] = COLUMNS_BLOCKDIM_X;
	localWorkSizeCols[1] = COLUMNS_BLOCKDIM_Y;

	globalWorkSizeCols[0] = col;
	globalWorkSizeCols[1] = row / COLUMNS_RESULT_STEPS;

	errCode = clEnqueueNDRangeKernel(gpu->clCommandQue, kernelConvRows, 2, NULL, globalWorkSizeRows, localWorkSizeRows, 0, NULL, NULL);
	gpu->checkErr("clEnqueueNDRangeKernel", errCode);


	errCode = clEnqueueNDRangeKernel(gpu->clCommandQue, kernelConvCols, 2, NULL, globalWorkSizeCols, localWorkSizeCols, 0, NULL, NULL);
	gpu->checkErr("clEnqueueNDRangeKernel", errCode);

	errCode = clEnqueueReadBuffer(gpu->clCommandQue, buf_output, CL_TRUE, 0, mem_size, calculated->mem, 0, NULL, NULL);
	gpu->checkErr("clEnqueueReadBuffer", errCode);


	clFinish(gpu->clCommandQue);

	clReleaseMemObject(buf_input);
	clReleaseMemObject(buf_output);
	clReleaseMemObject(buf_temp);
	clReleaseMemObject(buf_filter);

	clReleaseKernel(kernelConvRows);
	clReleaseKernel(kernelConvCols);

	return true;
}
