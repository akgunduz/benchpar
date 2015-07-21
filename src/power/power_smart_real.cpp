//
//  power.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "power_smart_real.h"

Power_SMARTREAL::Power_SMARTREAL() {

}

Power_SMARTREAL::~Power_SMARTREAL() {

}

bool Power_SMARTREAL::readStatus(bool &state) {

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

bool Power_SMARTREAL::toggleStart(bool prevState) {

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

bool Power_SMARTREAL::read_before() {

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

	initialized = true;

	return true;
}

double Power_SMARTREAL::read_after() {

	if (!initialized) {
		return 0;
	}

	bool res = toggleStart(true);

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
			printf("Problem with Smart device read status\n");
			return false;
		}

		if (buffer[0] == REQUEST_DATA) {
			break;
		}
	}

	hid_close(device);

	if (buffer[0] == REQUEST_DATA) {
		sscanf((char *)&buffer[24], "%lfW", &consumed);
		consumed *= H_TO_S;

		return consumed;
	}

	printf("Problem with Smart device read data\n");

	return 0;
}
