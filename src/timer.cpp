//
//  timer.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "common.h"

double Timer::getTime() {

	struct timeval refTime;
	int res = gettimeofday(&refTime, nullptr);
	if (res != 0) {
		printf("Can not get time!!!\n");
		return 0;
	}

	return refTime.tv_sec * 1000 + (double)refTime.tv_usec / 1000;

}

double Timer::snapshot() {

	startTime = getTime();
	return startTime;
}

double Timer::getdiff() {

	if (startTime == 0) {
		printf("Take snaphot first!\n");
		return 0;
	}
	return getTime() - startTime;
}
