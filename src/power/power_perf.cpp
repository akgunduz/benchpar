//
//  power.cpp
//  MatrixCalculator
//
//  Created by Haluk Akgunduz on 01/09/14.
//  Copyright (c) 2014 Haluk Akgunduz. All rights reserved.
//

#include "power_perf.h"

Power_PERF::Power_PERF() {

}

Power_PERF::~Power_PERF() {

}

bool Power_PERF::open_perf() {

	FILE *ff = fopen(PERF_PATH, "r");
	if (ff == NULL) {
		printf("No perf_event rapl support found (requires Linux 3.14)\n");
		return false;
	}

	int res = fscanf(ff, "%d", &perf_type);
        if (res == EOF) {
            	fclose(ff);
		printf("File Read Error happened!");
                return false;
        }
	fclose(ff);

	char filename[PATH_LENGTH];

	for (int j = 0; j < NUM_RAPL_DOMAINS; j++) {

		res = sprintf(filename, PERF_EVENT, rapl_domain_names[j]);
                if (res < 0) {
                    printf("sprintf Error happened!");
                }

		FILE *f = fopen(filename, "r");

		if (f != NULL) {
			res = fscanf(f, "event=%x", &perf_config[j]);
                        if (res == EOF) {
                                fclose(f);
                                printf("File Read Error happened!");
                                continue;
                        }
			fclose(f);
		} else {
			continue;
		}

		sprintf(filename, PERF_SCALE, rapl_domain_names[j]);

		f = fopen(filename, "r");
		if (f != NULL) {
			res = fscanf(f, "%lf", &perf_scale[j]);
                        if (res == EOF) {
                                fclose(f);
                                printf("File Read Error happened!");
                                continue;
                        }
			fclose(f);
		}

		sprintf(filename, PERF_UNIT, rapl_domain_names[j]);
		f = fopen(filename, "r");

		if (f != NULL) {
			res = fscanf(f, "%s", perf_units[j]);
                        if (res == EOF) {
                                fclose(f);
                                printf("File Read Error happened!");
                                continue;
                        }
			fclose(f);
		}

	}

	return true;
}

bool Power_PERF::start_perf(int core, int domain, struct perf_event_attr *attr) {

	int id = core * NUM_RAPL_DOMAINS + domain;

	fd[id] = syscall(__NR_perf_event_open, attr, -1, core, -1, 0);

	if (fd[id] < 0) {
		if (errno == EACCES) {
			printf("Permission denied; run as root or adjust paranoid value\n");
			return false;
		} else {
			printf("error opening: %s\n", strerror(errno));
			return false;
		}
	}

	return true;

}

long long Power_PERF::read_perf(int id) {

	long long value;

	int res = read(fd[id], &value, 8);
        if (res == -1) {
            printf("Read Error happened!");
            return 0;
        }

	return value;

}

void Power_PERF::close_perf(int id) {

	close(fd[id]);

}


bool Power_PERF::read_before() {

	open_perf();

	char filename[PATH_LENGTH];

	struct perf_event_attr attr;
	memset(&attr, 0, sizeof(perf_event_attr));

	for (int i = 0; i < core_count; i++) {

		for (int j = 0; j < NUM_RAPL_DOMAINS; j++) {

			attr.type = perf_type;
			attr.config = perf_config[j];

			start_perf(i, j, &attr);

		}
	}

	return true;

}

double Power_PERF::read_after() {

	long long value;

	perf_consumed = 0;

	for (int i = 0; i < core_count; i++) {

		for (int j = 0; j < NUM_RAPL_DOMAINS; j++) {

			int id = i * NUM_RAPL_DOMAINS + j;

			if (fd[id] != -1) {
				value = read_perf(id);
				close_perf(id);

				perf_consumed += value * perf_scale[j];

			}
		}
	}

	return perf_consumed;
}
