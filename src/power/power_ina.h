//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __POWER_INA_H_
#define __POWER_INA_H_

#include "power.h"

#define DEV_SENSOR_ARM  "/dev/sensor_arm"
#define DEV_SENSOR_MEM  "/dev/sensor_mem"
#define DEV_SENSOR_KFC  "/dev/sensor_kfc"
#define DEV_SENSOR_G3D  "/dev/sensor_g3d"

#define INA231_IOCGREG      _IOR('i', 1, IOcreg *)
#define INA231_IOCSSTATUS   _IOW('i', 2, IOcreg *)
#define INA231_IOCGSTATUS   _IOR('i', 3, IOcreg *)

#define INA231_WAIT 250

enum SENSOR_INA {
	SENSOR_ARM,
	SENSOR_MEM,
	SENSOR_KFC,
	SENSOR_G3D,
	SENSOR_MAX
};

struct IOcreg {
	char name[20];
	unsigned int enable;
	unsigned int cur_uV;
	unsigned int cur_uA;
	unsigned int cur_uW;
};

struct Sensor {
	int  fd;
	char path[255];
	IOcreg data;
};

class Power_INA : public Power {

	Sensor sensor[SENSOR_MAX];
	pthread_mutex_t mMutex;
	pthread_cond_t mCond;
	pthread_t mThread;
	double consumed;
	bool initialized;

	double curWatt;

	bool getSensorDatas();
	static void* runMonitor(void *arg);

public:

	Power_INA();
	~Power_INA();

	bool read_before();
	double read_after();

};

#endif
