//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __CPU_H_
#define __CPU_H_

#include "common.h"

#define MAX_CORE_COUNT 		16

#define CPU_SANDYBRIDGE		42
#define CPU_SANDYBRIDGE_EP	45
#define CPU_IVYBRIDGE		58
#define CPU_IVYBRIDGE_EP	62
#define CPU_HASWELL			60
#define CPU_HASWELL_EP		63
#define CPU_BROADWELL		61

#define CPUINFO_PATH "/proc/cpuinfo"

enum PROCTYPE {
	PROC_UNKNOWN,
	PROC_INTEL,
	PROC_ARM
};

enum CORETYPE {
	CORE_UNKNOWN,
	CORE_SANDYBRIDGE,
	CORE_SANDYBRIDGE_EP,
	CORE_IVYBRIDGE,
	CORE_IVYBRIDGE_EP,
	CORE_HASWELL,
	CORE_HASWELL_EP,
	CORE_BROADWELL,
	CORE_A15,
	CORE_A7,
	CORE_A9,
	CORE_A8,
	CORE_MAX
};

class Core {
public:
	CORETYPE type;
};

class CPU {
public:
	int coreCount;
	int powerCoreCount;
	PROCTYPE type;
	Core cores[MAX_CORE_COUNT];

	const char names[CORE_MAX][30] = {
			"CORE_UNKNOWN",
			"CORE_SANDYBRIDGE", "CORE_SANDYBRIDGE_EP",
			"CORE_IVYBRIDGE", "CORE_IVYBRIDGE_EP",
			"CORE_HASWELL", "CORE_HASWELL_EP",
			"CORE_BROADWELL",
			"CORE_A15", "CORE_A7", "CORE_A9", "CORE_A8"
	};

	CPU();
	bool detectCPU();
	CORETYPE getCoreType();
};


#endif //__CPU_H_
