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

	enum SEQTYPE seqID = SEQTYPE_NONE;
	enum MULTYPE modeID = MULTYPE_CPU_STD;
	enum MULTYPE sanityID = MULTYPE_MAX;

public:

	MatrixApp(CPU *c, GPU *g, Power *pw);
	~MatrixApp();

	virtual bool loadGPUKernel();
	virtual void unLoadGPUKernel();

	Matrix* calculate(Matrix *A, Matrix *B, int modeID, bool print, int repeat);

	bool process(char fileInputs[][255]);
	bool processDir(const char path[255]);

	virtual bool run(int argc, const char argv[][ARGV_LENGTH]);

};

#endif
