//
//  matrixapp.h
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#ifndef __MATRIXAPP_H_
#define __MATRIXAPP_H_

#include "app.h"
#include "matrix.h"

class MatrixApp : public App {

public:

	MatrixApp(CPU *c, GPU *g, Power *pw, const char *path);
	~MatrixApp();

	virtual bool loadGPUKernel();
	virtual void unLoadGPUKernel();

	Matrix* calculate(Matrix *A, Matrix *B, int modeID, int repeat);

	virtual int getFuncModeCount(FUNCTYPE);
	virtual bool creator(uint32_t, uint32_t, uint32_t);

	bool process(char fileInputs[PATH_MAX]);
	bool processDir(const char path[PATH_MAX]);
	bool processList(char fileInputs[][PATH_MAX], int size);

	virtual bool run(int argc, const char argv[][ARGV_LENGTH]);

};

#endif
