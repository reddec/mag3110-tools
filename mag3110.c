/* *
 * This file contains implementation of API for basic communication with LPS 332 sensor
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

typedef enum {
    MAG3110_ADDR_DR_STATUS = 0x00, // Data ready status per axis
    MAG3110_ADDR_OUT_X_MSB = 0x01, // Bits [15:8] of X measurement
    MAG3110_ADDR_OUT_X_LSB = 0x02, // Bits [7:0] of X measurement
    MAG3110_ADDR_OUT_Y_MSB = 0x03, // Bits [15:8] of Y measurement
    MAG3110_ADDR_OUT_Y_LSB = 0x04, // Bits [7:0] of Y measurement
    MAG3110_ADDR_OUT_Z_MSB = 0x05, // Bits [15:8] of Z measurement
    MAG3110_ADDR_OUT_Z_LSB = 0x06, // Bits [7:0] of Z measurement
    MAG3110_ADDR_WHO_AM_I = 0x07, // Device ID Number
    MAG3110_ADDR_SYSMOD = 0x08, // Current System Mode
    MAG3110_ADDR_OFF_X_MSB = 0X09, // Bits [14:7] of user X offset
    MAG3110_ADDR_OFF_X_LSB = 0X0A, // Bits [6:0] of user X offset
    MAG3110_ADDR_OFF_Y_MSB = 0X0B, // Bits [14:7] of user Y offset
    MAG3110_ADDR_OFF_Y_LSB = 0X0C, // Bits [6:0] of user Y offset
    MAG3110_ADDR_OFF_Z_MSB = 0X0D, // Bits [14:7] of user Z offset
    MAG3110_ADDR_OFF_Z_LSB = 0X0E, // Bits [6:0] of user Z offset
    MAG3110_ADDR_DIE_TEMP = 0X0F, // Temperature, signed 8 bits in C
    MAG3110_ADDR_CTRL_REG1 = 0X10, // Operation modes
    MAG3110_ADDR_CTRL_REG2 = 0X11, // Operation modes
} MAG3110_ADDR;

static const uint8_t MAG3110_ID = 0xC4;

// Helper function to read I2C register
ssize_t read_at(int fd, uint8_t addr, void *buffer, size_t size) {
    if (write(fd, &addr, 1) <= 0) {
        return -1;
    }
    return read(fd, buffer, size);
}

bool mag3110_check_device(int fd) {
    uint8_t id;
    if (read_at(fd, MAG3110_ADDR_WHO_AM_I, &id, 1) < 0) {
        return false;
    }
    return id == MAG3110_ID;
}

int8_t mag3110_temperature(int fd) {
    int8_t value;
    if (read_at(fd, MAG3110_ADDR_DIE_TEMP, &value, sizeof(value)) < 0) {
        return -127;
    };
    return value;
}

uint8_t mag3110_status(int fd) {
    uint8_t value = 0;
    read_at(fd, MAG3110_ADDR_DR_STATUS, &value, sizeof(value));
    return value;
}


bool mag3110_status_ready(uint8_t status) {
    return (status & 0b00001000) > 0;
}

bool mag3110_magnitude(int fd, int16_t *x, int16_t *y, int16_t *z) {
    uint8_t lm[2];
    if (read_at(fd, MAG3110_ADDR_OFF_X_LSB, lm, 1) < 0)
        return false;
    if (read_at(fd, MAG3110_ADDR_OFF_X_MSB, lm + 1, 1) < 0)
        return false;
    *x = (int16_t) ((lm[0] << 8) | lm[1]);
    if (read_at(fd, MAG3110_ADDR_OFF_Y_LSB, lm, 1) < 0)
        return false;
    if (read_at(fd, MAG3110_ADDR_OFF_Y_MSB, lm + 1, 1) < 0)
        return false;
    *y = (int16_t) ((lm[0] << 8) | lm[1]);
    if (read_at(fd, MAG3110_ADDR_OFF_Z_LSB, lm, 1) < 0)
        return false;
    if (read_at(fd, MAG3110_ADDR_OFF_Z_MSB, lm + 1, 1) < 0)
        return false;
    *z = (int16_t) ((lm[0] << 8) | lm[1]);
    return true;
}

bool mag3110_configure(int fd, uint8_t data_rate, uint8_t over_sampling_ratio) {
    uint8_t mode = data_rate << 5;
    over_sampling_ratio = over_sampling_ratio << 6;//Fill zero =)
    mode |= (over_sampling_ratio >> 3) | 0b0000001;
    uint8_t payload[2] = {MAG3110_ADDR_CTRL_REG1, mode};
    return write(fd, payload, sizeof(payload)) == ((ssize_t)
            sizeof(payload));
}

int mag3110_open_device(const char *device, uint8_t address) {
    int bus_fd = open(device, O_RDWR);
    if (bus_fd < 0) {
        return -1;
    }
    if (ioctl(bus_fd, I2C_SLAVE, address) < 0) {
        close(bus_fd);
        return -1;
    }
    return bus_fd;
}