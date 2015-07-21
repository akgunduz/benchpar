//
// Created by Haluk AKGUNDUZ on 15/07/15.
// Copyright (c) 2015 Haluk Akgunduz. All rights reserved.
//


#ifndef __POWER_PERF_H_
#define __POWER_PERF_H_

#include "power.h"
#include <linux/perf_event.h>

#define PERF_PATH "/sys/bus/event_source/devices/power/type"
#define PERF_EVENT "/sys/bus/event_source/devices/power/events/%s"
#define PERF_SCALE "/sys/bus/event_source/devices/power/events/%s.scale"
#define PERF_UNIT "/sys/bus/event_source/devices/power/events/%s.unit"

//#define ALL_DOMAINS_ENABLED

#ifdef ALL_DOMAINS_ENABLED
#define NUM_RAPL_DOMAINS 4
#else
#define NUM_RAPL_DOMAINS 1
#endif

class Power_PERF : public Power {

	int fd[MAX_CORE_COUNT * NUM_RAPL_DOMAINS];

	const char rapl_domain_names[NUM_RAPL_DOMAINS][30] = {
			"energy-cores",
#ifdef ALL_DOMAINS_ENABLED
			/*No need below domains*/
			"energy-gpu",
			"energy-pkg",
			"energy-ram",
#endif
	};

	int perf_type;
	double perf_consumed = 0.0;
	int perf_config[NUM_RAPL_DOMAINS];
	double perf_scale[NUM_RAPL_DOMAINS];
	char perf_units[NUM_RAPL_DOMAINS][PATH_LENGTH];

	bool open_perf();
	bool start_perf(int core, int domain, struct perf_event_attr *attr);
	long long read_perf(int id);
	void close_perf(int id);

public:

	Power_PERF();
	~Power_PERF();

	bool read_before();
	double read_after();

};

#endif
