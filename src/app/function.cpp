//
// Created by Haluk AKGUNDUZ on 13/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//

#include "function.h"

FuncList::FuncList() { 

}

FuncList::~FuncList() { 
        for (int i = 0; i < kernelCount; i++) {
                clReleaseKernel(kernels[i]);
        }
}

FuncList* FuncList::createArray(int size, GPU* gpu) {
        
        FuncList* list = new FuncList[size];
        for (int i = 0; i < size; i++) {
                list[i].gpu = gpu;
        }
        
        return list;
}

void FuncList::set(const char *id, fFuncs func, int kernelCount, int argCount, const char *kernelid[], int argument[]) {
        
        cl_int errCode;
        
        this->id = id;
        
        if (kernelCount > MAX_ARGUMENT) {
            kernelCount = MAX_ARGUMENT;
        }
        
        if (argCount > MAX_ARGUMENT) {
            argCount = MAX_ARGUMENT;
        }

        this->kernelCount = kernelCount;
        this->argCount = argCount;

        for (int i = 0; i < kernelCount; i++) {
                
            if (kernelid != NULL) {
                strcpy(kernelID[i], kernelid[i]);
                kernels[i] = clCreateKernel(gpu->clProgram, kernelid[i], &errCode);
                gpu->checkErr("clCreateKernel", errCode);
            }

        }
        
        for (int i = 0; i < argCount; i++) {
            
            if (argument != NULL) {
                this->argument[i] = argument[i];
            } else {
                this->argument[i] = 0;
            }

        }

        this->f = func;
};

Function::Function(GPU *gpu) {
        this->gpu = gpu;
}

Function::~Function() {

}

void Function::consoleOut(int offset, int max) {

	printf("Printing Out in Size: %d\n", (int)size);

	for (int i = offset; i < size; i++) {

                if (i == max) {
                        printf("\n");
                        return;
                }
                
		printf("%f, ", mem[i]);
	}
}

