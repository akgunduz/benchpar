//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __POWER_SMART_REAL_H_
#define __POWER_SMART_REAL_H_

#include "power.h"
#include "hidapi.h"

#define MAX_STR 65
#define SLEEP_TIME 10000
#define H_TO_S 3600

#define REQUEST_DATA        0x37
#define REQUEST_STARTSTOP   0x80
#define REQUEST_STATUS      0x81
#define REQUEST_ONOFF       0x82
#define REQUEST_VERSION     0x83

class Power_SMARTREAL : public Power {

	double consumed;
	bool initialized;

	hid_device *device;

	double curWatt;

	bool getSensorDatas(bool &initial);

public:

	Power_SMARTREAL();
	~Power_SMARTREAL();

	bool readStatus(bool &state);
	bool toggleStart(bool prevState);

	bool read_before();
	double read_after();

};

#endif
