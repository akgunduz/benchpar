//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __scan_H_
#define __scan_H_

#include "gpu.h"

class Scan;

typedef bool (Scan::*fScanner)(Scan *, GPU *);

enum SCANTYPE {
	SCANTYPE_CPU_STD,
#ifndef C15
	SCANTYPE_CPU_AVX,
	SCANTYPE_CPU_SSE,
	SCANTYPE_CPU_OMP_SSE,
	SCANTYPE_CPU_OMP_SSEp2_SSEp1,
#endif
	SCANTYPE_GPU_STD,
	SCANTYPE_MAX,
};

struct Scanners {

	const char id[255];
	const char kernelid[255];
	int divider;
	fScanner f;
};

class Scan {

	uint32_t size;
	size_t mem_size;
	float *mem;

	bool allocMem(uint32_t size);

public:

	Scanners scanners[SCANTYPE_MAX] = {
			{"SCANTYPE_CPU_STD", "", 1, &Scan::scanCPU_STD},
#ifndef C15
			{"SCANTYPE_CPU_AVX", "", 1, &Scan::scanCPU_AVX},
			{"SCANTYPE_CPU_SSE", "", 1, &Scan::scanCPU_SSE},
			{"SCANTYPE_CPU_OMP_SSE", "", 1, &Scan::scanCPU_OMP_SSE},
			{"SCANTYPE_CPU_OMP_SSEp2_SSEp1", "", 1, &Scan::scanCPU_OMP_SSEp2_SSEp1},
#endif
			{"SCANTYPE_GPU_STD", "", 1, &Scan::scanGPU_STD},
	};

	Scan(uint32_t size, bool prepare = false);
	Scan(std::string path);
	~Scan();

	void printOut();
	bool printToFile(uint32_t);

	bool scanCPU_STD(Scan *, GPU *);

#ifndef C15
	bool scanCPU_AVX(Scan *, GPU *);
	bool scanCPU_SSE(Scan *, GPU *);
	bool scanCPU_OMP_SSE(Scan *, GPU *);
	bool scanCPU_OMP_SSEp2_SSEp1(Scan *, GPU *);
#endif

	bool scanGPU_STD(Scan *, GPU *);

	bool scanGPU(Scan *calculated, enum SCANTYPE scanType, GPU *);

	void create(uint32_t size);
	bool compare(Scan *);
	bool check();

	uint32_t getSize();
};


#endif //__scan_H_
