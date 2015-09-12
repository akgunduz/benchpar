//
//  app.cpp
//  Application Main Class
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "app.h"
#include "matrixapp.h"
#include "convapp.h"
#include "scanapp.h"

App *App::newInstance(APP_MODES mode, CPU *cpu, GPU *gpu, Power *power) {

	App *app = NULL;

	switch(mode) {
		case MATRIX_MODE:
		default:
			app = new MatrixApp(cpu, gpu, power);
			break;
		case SCAN_MODE:
			app = new ScanApp(cpu, gpu, power);
			break;
		case CONV_MODE:
			app = new ConvApp(cpu, gpu, power);
			break;
	}

	return app;
}

App::App(CPU *c, GPU *g, Power *pw):
		cpu(c), gpu(g), power(pw) {

	print_enabled = false;
	repeat = 1;

};

App::~App() {

}

void App::setPrintState(bool state) {

	print_enabled = state;
}

void App::setRepeat(int count) {

	repeat = count;
}

void App::setModes(int modeID, int seqID, int sanityID) {

	if (modeID < getFuncModeCount(FUNCTYPE_ALL)) {
		this->modeID = modeID;
	}

	this->seqID = seqID;

	if (sanityID < getFuncModeCount(FUNCTYPE_ALL) && seqID == SEQTYPE_NONE && sanityID != modeID) {
		this->sanityID = sanityID;
	}
}
