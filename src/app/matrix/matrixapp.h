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
	MatrixApp(CPU *c, GPU *g, Power *pw):
			App(c, g, pw){};

	Matrix* process(Matrix *A, Matrix *B, int modeID, bool print, int repeat);

	virtual bool init(int argc, const char *argv[]);
};

#endif
