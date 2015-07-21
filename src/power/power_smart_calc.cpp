//
//  power.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "power_smart_calc.h"

Power_SMARTCALC::Power_SMARTCALC() {

}

Power_SMARTCALC::~Power_SMARTCALC() {

}

bool Power_SMARTCALC::readStatus(bool &state) {

	uint8_t buffer[MAX_STR];
	memset(buffer, 0, MAX_STR);
	buffer[0] = 0x00;
	buffer[1] = REQUEST_STATUS;

	if (hid_write(device, buffer, sizeof(buffer)) == -1) {
		printf("Problem with Smart device write status\n");
		return false;
	}

	int count = 10;

	while(count--) {

		usleep(SLEEP_TIME);

		if (hid_read(device, buffer, sizeof(buffer)) == -1) {
			printf("Problem with Smart device read status\n");
			return false;
		}

		if (buffer[0] == REQUEST_STATUS) {
			break;
		}
	}

	if (buffer[0] == REQUEST_STATUS) {

		state = buffer[1] == 0x01;
		return true;
	}

	printf("Problem with Smart device read status data\n");

	return false;
}

bool Power_SMARTCALC::toggleStart(bool prevState) {

	uint8_t buffer[MAX_STR];
	memset(buffer, 0, MAX_STR);
	buffer[0] = 0x00;
	buffer[1] = REQUEST_STARTSTOP;

	if (hid_write(device, buffer, sizeof(buffer)) == -1) {
		printf("Problem with Smart device write start\n");
		return false;
	}
/*
	usleep(SLEEP_TIME);

	bool newState = false;

	bool res = readStatus(newState);
	if (!res) {
		return false;
	}

	if (newState != prevState) {
		return true;
	}

	printf("Problem with Smart device read start data\n");

	return false;
	*/

	return true;
}

bool Power_SMARTCALC::read_before() {

	initialized = false;

	consumed = 0.0;

	device = hid_open(0x04d8, 0x003f, NULL);

	if (device == NULL) {
		printf("Problem with Smart device open\n");
	}

	hid_set_nonblocking(device, true);

	bool state = false;
	bool hidres = readStatus(state);
	if (!hidres) {
		hid_close(device);
		return false;
	}

	if (!state) {
		hidres = toggleStart(state);
		if (!hidres) {
			hid_close(device);
			return false;
		}
	}

	int res = pthread_mutex_init(&mMutex, NULL);
	if (res) {
		printf("Smart Mutex init failed\n");
		hid_close(device);
		return false;
	}

	res = pthread_cond_init (&mCond, NULL);
	if (res) {
		printf("Smart Condition init fail\n");
		hid_close(device);
		pthread_mutex_destroy(&mMutex);
		return false;
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	int pthr = pthread_create(&mThread, &attr, runMonitor, this);
	pthread_attr_destroy(&attr);
	if (pthr) {
		printf("Problem with Smart thread\n");
		hid_close(device);
		pthread_cond_destroy(&mCond);
		pthread_mutex_destroy(&mMutex);
		return false;
	}

	initialized = true;

	return true;
}

double Power_SMARTCALC::read_after() {

	if (!initialized) {
		return 0;
	}

	pthread_mutex_lock(&mMutex);
	pthread_cond_signal(&mCond);
	pthread_mutex_unlock(&mMutex);
	pthread_join(mThread, NULL);

	bool res = toggleStart(true);

	hid_close(device);

	pthread_cond_destroy(&mCond);
	pthread_mutex_destroy(&mMutex);

	return consumed;
}

#if 0
bool Power_SMART::getSensorDatas() {

	uint8_t buffer[MAX_STR];
	memset(buffer, 0, MAX_STR);
	buffer[0] = 0x00;
	buffer[1] = REQUEST_DATA;

	if (hid_write(device, buffer, sizeof(buffer)) == -1) {
		printf("Problem with Smart device write data\n");
		return false;
	}

	int count = 10;

	while(count--) {

		usleep(SLEEP_TIME);

		if (hid_read(device, buffer, sizeof(buffer)) == -1) {
			printf("Problem with Smart device read data\n");
			return false;
		}

		if (buffer[0] == REQUEST_DATA) {
			break;
		}
	}

	if (buffer[0] == REQUEST_DATA) {

		printf("%s\n", &buffer[2]);
		return true;
	}

	printf("Problem with Smart device read data\n");

	return false;

}
#else
bool Power_SMARTCALC::getSensorDatas(bool &initial) {

	uint8_t buffer[MAX_STR];
	memset(buffer, 0, MAX_STR);

	if (!initial) {
		if (hid_read(device, buffer, sizeof(buffer)) == -1) {
			printf("Problem with Smart device read data\n");
			return false;
		}

		if (buffer[0] == REQUEST_DATA) {
                	sscanf((char *)&buffer[18], "%lfW", &curWatt);
		}
	} else {
		initial = false;
	}

	buffer[0] = 0x00;
	buffer[1] = REQUEST_DATA;

	if (hid_write(device, buffer, sizeof(buffer)) == -1) {
		printf("Problem with Smart device write data\n");
		return false;
	}

	return true;

}
#endif

void* Power_SMARTCALC::runMonitor(void *arg) {

	struct timespec to;
	struct timeval now;

	Power_SMARTCALC *power = (Power_SMARTCALC*) arg;

	bool thread_started = true;
	bool initial = true;

	while(thread_started) {

		pthread_mutex_lock(&power->mMutex);

		gettimeofday(&now,NULL);

		long long target_ms = now.tv_sec * 1000 + now.tv_usec / 1000 + SMART_WAIT;

		to.tv_sec = target_ms / 1000;
		to.tv_nsec = (target_ms % 1000) * 1000000;

		int res = pthread_cond_timedwait(&power->mCond, &power->mMutex, &to);

		switch(res) {

			case 0:
				thread_started = false;
				break;

			case ETIMEDOUT:
				thread_started = power->getSensorDatas(initial);
				power->consumed += ((double)SMART_WAIT / 1000.0) * power->curWatt;
				break;

			default:
				printf("Smart Error happened : %d\n", res);
				break;
		}

		pthread_mutex_unlock(&power->mMutex);

	}

	return NULL;
}
