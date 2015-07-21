//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __POWER_MSR_H_
#define __POWER_MSR_H_

#include "power.h"

#define MSR_PATH "/dev/cpu/%d/msr"
#define MSR_RAPL_POWER_UNIT		0x606
#define MSR_PKG_ENERGY_STATUS		0x611
#define MSR_PP0_ENERGY_STATUS		0x639
#define MSR_PP1_ENERGY_STATUS		0x641

class Power_MSR : public Power {

	int fd[MAX_CORE_COUNT];

	double package_before, package_after;
	double cpu_energy_units[16];
	double pp0_before = 0.0, pp0_after;
	double pp1_before = 0.0, pp1_after;

	bool open_msr(int core);
	long long read_msr(int core, int which);
	void close_msr(int core);

public:

	Power_MSR();
	~Power_MSR();

	bool read_before();
	double read_after();

};

#endif
