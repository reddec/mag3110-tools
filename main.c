/* *
 * This file contains simple CLI utility for reading data from MAG 3110 sensor
 *
 * This file is part of mag3110-tools.
 *
 * mag3110-tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mag3110-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mag3110-tools.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2016 Baryshnikov Alexander <dev@baryshnikov.net>
 *
 * Please feel free to contact with author
 */
#include "mag3110.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <inttypes.h>
#include <time.h>

extern char *optarg;
extern int optind;

void summary(const char *command) {
    printf("MAG3110 CLI reader 1.0.0\n");
    printf("\n");
    printf("Usage: %s [-d data rate][-o oversampling][-i interval][-l name] <device> <address>\n", command);
    printf("\n");
    printf("  device - path of I2C device (like /dev/i2c-1)\n");
    printf("  device - address of sensor in device in HEX (like 0x52)\n");
    printf("\n");
    printf("  -d number   - positive or zero value of data rate in range [0;8)\n");
    printf("  -o number   - positive or zero value of oversampling ratio in range [0;4)\n");
    printf("  -i number   - positive or zero value of scanning interval in milliseconds\n");
    printf("  -l name     - use output format as InfluxDB line format with this name as measurment\n");
    printf("\n");
    printf("Version: 1.0.0\n");
    printf("Author: Baryshnikov Alexander <dev@baryshnikov.net>\n");
}

int main(int argc, char **argv) {
    uint8_t data_rate = 7;
    uint8_t over_sampling_ratio = 0;
    bool influxdb_format = false;
    const char *influxdb_name = "mag3110";
    int rez = 0;
    struct timespec interval;
    interval.tv_sec = 0;
    interval.tv_nsec = 100 * 1000;
    while ((rez = getopt(argc, argv, "d:o:hi:l")) != -1) {
        switch (rez) {
            case 'd': {
                int value = atoi(optarg);
                if (value < 0) {
                    fprintf(stderr, "data rate must have positive or zero value\n");
                    return 1;
                }
                if (value > 7) {
                    fprintf(stderr, "data rate must be less then 8\n");
                    return 1;
                }
                data_rate = (uint8_t) value;
                break;
            }
            case 'i': {
                int value = atoi(optarg);
                if (value < 0) {
                    fprintf(stderr, "interval must have positive or zero value\n");
                    return 1;
                }
                interval.tv_sec = ((uint32_t) value) / 1000;
                interval.tv_sec = (((uint32_t) value) - interval.tv_sec * 1000) * 1000;
                break;
            }
            case 'o': {
                int value = atoi(optarg);
                if (value < 0) {
                    fprintf(stderr, "over-sampling ratio must be positive or zero value\n");
                    return 1;
                }
                if (value > 3) {
                    fprintf(stderr, "over-sampling ratio must be less then 4\n");
                    return 1;
                }
                over_sampling_ratio = (uint8_t) value;
                break;
            }
            case 'l': {
                influxdb_name = optarg;
                influxdb_format = true;
                break;
            }
            case 'h':
                summary(argv[0]);
                return 0;
            default:
                summary(argv[0]);
                return 1;
        };
    };
    if (optind >= argc) {
        fprintf(stderr, "required parameter endpoint not provided\n");
        summary(argv[0]);
        return 1;
    }
    const char *endpoint = argv[optind];
    ++optind;
    if (optind >= argc) {
        fprintf(stderr, "required parameter address not provided\n");
        summary(argv[0]);
        return 1;
    }
    const uint8_t address = (uint8_t) strtol(argv[optind], NULL, 0);
    int sensor_fd = mag3110_open_device(endpoint, address);
    if (sensor_fd < 0) {
        perror("open bus");
        return 2;
    }
    if (!mag3110_check_device(sensor_fd)) {
        fprintf(stderr, "device is not MAG3110\n");
        return 3;
    }
    if (!mag3110_configure(sensor_fd, data_rate, over_sampling_ratio)) {
        perror("configure");
        return 4;
    }
    uint8_t status;
    bool skipped;
    while (1) {
        skipped = true;
        status = mag3110_status(sensor_fd);

        if (mag3110_status_ready(status)) {
            struct timeval now;
            gettimeofday(&now, NULL);
            uint64_t nano_time = (uint64_t) (now.tv_sec) * (1000 * 1000 * 1000) + now.tv_usec * 1000;
            int16_t x, y, z;
            int8_t temp = mag3110_temperature(sensor_fd);
            mag3110_magnitude(sensor_fd, &x, &y, &z);
            if (!influxdb_format) {
                printf("temp %i %s %" PRIu64 "\n", temp, "c", nano_time);
                printf("magnetism-x %i %s %" PRIu64 "\n", x, "microtesla", nano_time);
                printf("magnetism-y %i %s %" PRIu64 "\n", y, "microtesla", nano_time);
                printf("magnetism-z %i %s %" PRIu64 "\n", z, "microtesla", nano_time);
            } else {
                printf("%s temp=%i,magnetism-x=%i,magnetism-y=%i,magnetism-z=%i %" PRIu64 "\n",
                       influxdb_name,
                       temp, x, y, z,
                       nano_time);
            }
            skipped = false;
        }
        if (!skipped) {
            fflush(stdout);
            fflush(stderr);
        } else {
            if (nanosleep(&interval, NULL) < 0) break;
        }
    }
    return 0;
}
