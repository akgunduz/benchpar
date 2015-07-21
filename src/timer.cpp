//
//  timer.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "common.h"

double Timer::getTime() {

	if (!enabled) {
		return 0;
	}

	struct timeval refTime;
	int res = gettimeofday(&refTime, nullptr);
	if (res != 0) {
		printf("Can not get time!!!\n");
		return 0;
	}

	return refTime.tv_sec + (double)refTime.tv_usec / 1000000;

}

double Timer::snapshot() {

	if (!enabled) {
		return 0;
	}

	startTime = getTime();
	return startTime;
}

double Timer::getdiff() {

	if (!enabled) {
		return 0;
	}

	if (startTime == 0) {
		printf("Take snaphot first!\n");
		return 0;
	}
	return getTime() - startTime;
}
