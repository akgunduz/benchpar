//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __POWER_SMART_CALC_H_
#define __POWER_SMART_CALC_H_

#include "power.h"
#include "hidapi.h"

#define SMART_WAIT 250
#define MAX_STR 65
#define SLEEP_TIME 10000

#define REQUEST_DATA        0x37
#define REQUEST_STARTSTOP   0x80
#define REQUEST_STATUS      0x81
#define REQUEST_ONOFF       0x82
#define REQUEST_VERSION     0x83

class Power_SMARTCALC : public Power {

	pthread_mutex_t mMutex;
	pthread_cond_t mCond;
	pthread_t mThread;
	double consumed;
	bool initialized;

	hid_device *device;

	double curWatt;

	bool getSensorDatas(bool &initial);
	static void* runMonitor(void *arg);

public:

	Power_SMARTCALC();
	~Power_SMARTCALC();

	bool readStatus(bool &state);
	bool toggleStart(bool prevState);

	bool read_before();
	double read_after();

};

#endif
