//
//  power.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#ifdef __linux__
#include "power_msr.h"
#include "power_perf.h"
#include "power_ina.h"
#else
#include "power.h"
#endif
#include "power_smart_calc.h"
#include "power_smart_real.h"

Power::Power() {

}

Power::~Power() {

}

Power *Power::newInstance(POWER_MODES mode, int count) {

	Power *power;

	switch(mode) {
#ifdef __linux__
		case POWER_MSR:
			power = new Power_MSR();
			break;
		case POWER_PERF:
			power = new Power_PERF();
			break;
		case POWER_INA:
			power = new Power_INA();
			break;
#else
		case POWER_MSR:
		case POWER_PERF:
		case POWER_INA:
#endif
		case POWER_OFF:
		default:
			power = new Power();
			break;
		case POWER_SMARTCALC:
			power = new Power_SMARTCALC();
			break;
		case POWER_SMARTREAL:
			power = new Power_SMARTREAL();
			break;
	}

	power->core_count = count;
	power->mode = mode;

	return power;
}


POWER_MODES Power::getMode() {

	return mode;
}

bool Power::read_before() {

	return false;
}

double Power::read_after() {

	return 0;
}

const char *Power::getName() {

	return names[mode];
}
