//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __POWER_H_
#define __POWER_H_

#include "cpu.h"

enum POWER_MODES {
	POWER_OFF,
	POWER_MSR,
	POWER_PERF,
	POWER_INA,
	POWER_SMART,
	POWER_MAX,
};

class Power {

	POWER_MODES mode;
	const char names[POWER_MAX][PATH_LENGTH] = {
			"OFF", "MSR", "PERF", "INA", "SMART"
	};


protected:

	int core_count;
	Power();

public:

	static Power* newInstance(POWER_MODES, int);
	~Power();

	virtual bool read_before();
	virtual double read_after();

	POWER_MODES getMode();
	const char* getName();

};

#endif
