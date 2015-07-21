//
//  app.cpp
//  Application Main Class
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "app.h"
#include "matrixapp.h"

App *App::newInstance(APP_MODES mode, CPU *cpu, GPU *gpu, Power *power) {

	App *app = NULL;

	switch(mode) {
		case MATRIX_MODE:
		default:
			app = new MatrixApp(cpu, gpu, power);
			break;
	}

	return app;
}

void App::setPrintState(bool state) {

	print_enabled = state;
}

void App::setRepeat(int count) {

	repeat = count;
}
