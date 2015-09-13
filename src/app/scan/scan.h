//
// Created by Haluk AKGUNDUZ on 03/09/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __scan_H_
#define __scan_H_

#include "function.h"

enum SCANTYPE {
	SCANTYPE_CPU_STD,
#ifndef __ARM__
	SCANTYPE_CPU_AVX,
	SCANTYPE_CPU_SSE,
	SCANTYPE_CPU_OMP_SSE,
	SCANTYPE_CPU_OMP_SSEp2_SSEp1,
#endif
	SCANTYPE_GPU_STD,
	SCANTYPE_MAX,
};

class Scan : public Function {

	bool allocMem(uint32_t size);

public:

	Scan(uint32_t size, bool prepare = false);
	Scan(std::string path);
	~Scan();

	virtual void initFuncs();

	void printOut();
	bool printToFile(uint32_t);

	bool scanCPU_STD(Scan *, GPU *);

#ifndef __ARM__
	bool scanCPU_AVX(Scan *, GPU *);
	bool scanCPU_SSE(Scan *, GPU *);
	bool scanCPU_OMP_SSE(Scan *, GPU *);
	bool scanCPU_OMP_SSEp2_SSEp1(Scan *, GPU *);
#endif

	bool scanGPU_STD(Scan *, GPU *);

	bool scanGPU(Scan *calculated, int type, GPU *);

	void create(uint32_t size);
	bool compare(Scan *);
	bool check();

	uint32_t getSize();
};


#endif //__scan_H_
