//
//  power.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "power_ina.h"

Power_INA::Power_INA() {

}

Power_INA::~Power_INA() {

}


bool Power_INA::read_before() {

	initialized = false;

	consumed = 0.0;

	strcpy(sensor[SENSOR_ARM].path, DEV_SENSOR_ARM);
	strcpy(sensor[SENSOR_KFC].path, DEV_SENSOR_KFC);
	strcpy(sensor[SENSOR_G3D].path, DEV_SENSOR_G3D);
	strcpy(sensor[SENSOR_MEM].path, DEV_SENSOR_MEM);

	for (int i = 0; i < SENSOR_MAX; i++) {

		if ((sensor[i].fd = open(sensor[i].path, O_RDWR)) < 0) {
			printf("initINA231 Open Failed\n");
			return false;
		}

		if (ioctl(sensor[i].fd, INA231_IOCGSTATUS, &sensor[i].data) < 0) {
			printf("initINA231 IOCTL Error\n");
			return false;
		}

		if (sensor[i].data.enable == 0) {
			sensor[i].data.enable = 1;
			if (ioctl(sensor[i].fd, INA231_IOCSSTATUS, &sensor[i].data) < 0) {
				printf("initINA231 IOCTL Error\n");
				return false;
			}
		}

	}

	int res = pthread_mutex_init(&mMutex, NULL);
	if (res) {
		printf("INA231 Mutex init failed");
		return false;
	}

	res = pthread_cond_init (&mCond, NULL);
	if (res) {
		printf("INA231 Condition init fail");
		pthread_mutex_destroy(&mMutex);
		return false;
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	int pthr = pthread_create(&mThread, &attr, runMonitor, this);
	pthread_attr_destroy(&attr);
	if (pthr) {
		printf("Problem with INA231 thread");
		pthread_cond_destroy(&mCond);
		pthread_mutex_destroy(&mMutex);
		return false;
	}

	initialized = true;

	return true;
}

double Power_INA::read_after() {

	if (!initialized) {
		return 0;
	}

	pthread_mutex_lock(&mMutex);
	pthread_cond_signal(&mCond);
	pthread_mutex_unlock(&mMutex);
	pthread_join(mThread, NULL);

	for (int i = 0; i < SENSOR_MAX; i++) {

		if (sensor[i].fd > 0) {

			if (sensor[i].data.enable == 1) {
				sensor[i].data.enable = 0;
				if (ioctl(sensor[i].fd, INA231_IOCSSTATUS, &sensor[i].data) < 0) {
					printf("initINA231 IOCTL Error\n");
				}
			}

			close(sensor[i].fd);
		}
	}

	pthread_cond_destroy(&mCond);
	pthread_mutex_destroy(&mMutex);

	return consumed;
}

bool Power_INA::getSensorDatas() {

	for (int i = 0; i < SENSOR_MAX; i++) {
		if (sensor[i].fd > 0) {
			if (ioctl(sensor[i].fd, INA231_IOCGREG, &sensor[i].data) < 0) {
				printf("initINA231 IOCTL Error\n");
				return false;
			}
		} else {
			return false;
		}
	}

	curWatt = (double)(sensor[SENSOR_G3D].data.cur_uW + sensor[SENSOR_ARM].data.cur_uW +
			sensor[SENSOR_KFC].data.cur_uW + sensor[SENSOR_MEM].data.cur_uW) / 1000000;

	return true;
}

void* Power_INA::runMonitor(void *arg) {

	struct timespec to;
	struct timeval now;

	Power_INA *power = (Power_INA*) arg;

	bool thread_started = true;

	while(thread_started) {

		pthread_mutex_lock(&power->mMutex);

		gettimeofday(&now,NULL);

		uint64_t target_ms = (uint64_t) now.tv_sec * 1000 + now.tv_usec / 1000 + INA231_WAIT;

		to.tv_sec = (long) (target_ms / 1000);
		to.tv_nsec = (long) (target_ms % 1000) * 1000000;

		int res = pthread_cond_timedwait(&power->mCond, &power->mMutex, &to);

		switch(res) {

			case 0:
				thread_started = false;
				break;

			case ETIMEDOUT:
				thread_started = power->getSensorDatas();
				if (thread_started) {
					power->consumed += ((double) INA231_WAIT / 1000.0) * power->curWatt;
				}
				break;

			default:
				printf("initINA231 Error happened : %d\n", res);
				break;
		}

		pthread_mutex_unlock(&power->mMutex);

	}

	return NULL;
}
