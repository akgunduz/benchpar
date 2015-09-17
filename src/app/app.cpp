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

App *App::newInstance(APP_MODES mode, CPU *cpu, GPU *gpu, Power *power, const char *path) {

	App *app = NULL;

	switch(mode) {
		case MATRIX_MODE:
		default:
			app = new MatrixApp(cpu, gpu, power, path);
			break;
		case SCAN_MODE:
			app = new ScanApp(cpu, gpu, power, path);
			break;
		case CONV_MODE:
			app = new ConvApp(cpu, gpu, power, path);
			break;
	}

	return app;
}

App::App(CPU *c, GPU *g, Power *pw, const char *path):
		cpu(c), gpu(g), power(pw) {

	print_enabled = true;
	repeat = 1;
        setPath(path);
};

App::~App() {

}

void App::setModes(int modeID, int seqID, int sanityID, bool print_enabled, bool time_enabled, int repeat) {

	if (modeID < getFuncModeCount(FUNCTYPE_ALL)) {
		this->modeID = modeID;
	}

	this->seqID = seqID;

	if (sanityID < getFuncModeCount(FUNCTYPE_ALL) && seqID == SEQTYPE_NONE && sanityID != modeID) {
		this->sanityID = sanityID;
	}

	this->print_enabled = print_enabled;
	this->time_enabled = time_enabled;
	this->repeat = repeat;

}

bool App::printStart(const char* prefix, int fileID) {

	if (!print_enabled) {
		return true;
	}

	char dFile[255];
	char dMode[10];
	switch(seqID) {
		case SEQTYPE_ALL:
			strcpy(dMode, "ALL");
			break;
		case SEQTYPE_CPU:
			strcpy(dMode, "CPU");
			break;
		case SEQTYPE_GPU:
			strcpy(dMode, "GPU");
			break;
		default:
			sprintf(dMode, "M%02d", modeID);
			break;
	}

	char dDir[4];
	if (fileID == 0) {
		strcpy(dDir, "DIR");
	} else if (fileID == 0xFF) {
		strcpy(dDir, "MUL");
	} else {
		sprintf(dDir, "F%02d", fileID);
	}

	char dOS[4];
#if defined __APPLE__
	strcpy(dOS, "OSX");
#elif defined __ARM__
        switch(cpu->getCoreType()) {
                case CORE_A15:
                        strcpy(dOS, "XU3");
                        break;
                case CORE_A9:
                        strcpy(dOS, "_U3");
                        break;
                case CORE_A8:
                        strcpy(dOS, "BGL");
                        break;
                case CORE_A7:
                        strcpy(dOS, "RSP");
                        break;
                default:
                        strcpy(dOS, "ARM");
                        break;
        }
	
#else
	strcpy(dOS, "LNX");
#endif

	char dTime[100];
	time_t date = time(NULL);
	struct tm *ptm = gmtime(&date);
	sprintf(dTime, "T%02d_%02d_%d_%02d_%02d_%02d",
			ptm->tm_mday, ptm->tm_mon + 1, ptm->tm_year + 1900,
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

	sprintf(dFile, "%s/%s_%s_%s_%s_R%d_%s", getPath(), prefix, dOS, dMode, dDir, repeat, dTime);

	debugFile = fopen(dFile, "w");

	return debugFile != nullptr;
}

void App::printOut(const char* format, ...) {

	va_list arglist;

	va_start(arglist, format);
	if (print_enabled) {
		vfprintf(debugFile, format, arglist);
	}

	va_start (arglist, format);
	vprintf(format, arglist);

	va_end(arglist);
}

bool App::endCheck(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void App::setPath(const char *ref)
{
        const char *pos = strrchr(ref, '/');
        
        strcpy(path, "");
        
     //   strcpy(path, getcwd(NULL, 0));
     //   strcat(path, "/");
        strncat(path, ref, pos - ref + 1);
        printf("Working path : %s\n", path);
}

const char* App::getPath() 
{
        return path;
}


