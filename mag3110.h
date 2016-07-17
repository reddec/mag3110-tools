/* *
 * This file contains API for basic communication with MAG3110 sensor
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

#ifndef MAG3110_MAG3110_H
#define MAG3110_MAG3110_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Check that device is MAG3110 by reading WHO_AM_I register
 * @param fd file descriptor pointed to I2C device
 * @return true if response is correct otherwise false
 */
bool mag3110_check_device(int fd);

/**
* Configure sensor by setting custom data rate and over sampling ratio.
* For example: 1.2HZ is DR: 7 and OSR: 0
* Details see in table 32 in datasheet (https://www.nxp.com/files/sensors/doc/data_sheet/MAG3110.pdf)
* @param data_rate Rate for scanning values from 0 to 8 (incl)
* @param over_sampling_ratio Oversampling data ratio from 0 to 3 (incl)
*/
bool mag3110_configure(int fd, uint8_t data_rate, uint8_t over_sampling_ratio);

/**
* Get current status of sensor by STATUS_REG register. You can use helper function to check that status is
* by mag3110_status_ready
* @param fd file descriptor pointed to I2C device
* @return bit flags
*/
uint8_t mag3110_status(int fd);

/**
 * Get temperature in Celsium from sensor in range from -40 to +125
 * @param fd file descriptor pointed to I2C device
 * @return value of temperature or -127 if something wrong
 */
int8_t mag3110_temperature(int fd);

/**
 * Check status bitmask for flag when all XYZ values are ready to be read
 * @param status result of mag3110_status
 * @return true if registers are ready to be read
 */
bool mag3110_status_ready(uint8_t status);

/**
 * Get magnitude in micro-Tesla from sensor
 * @param fd file descriptor pointed to I2C device
 * @param x output value of magnitude force of X axes
 * @param y output value of magnitude force of Y axes
 * @param z output value of magnitude force of Z axes
 * @return true if all OK, otherwise false
 */
bool mag3110_magnitude(int fd, int16_t *x, int16_t *y, int16_t *z);

/**
 * Helper function to open MAG3110 sensor over I2C protocol
 * @param device name of I2C bus (ex: /dev/i2c-1)
 * @param address address in I2C of sensor (ex: 0x5C)
 * @return file descriptor pointed to sensor or -1
 */
int mag3110_open_device(const char *device, uint8_t address);

#endif //MAG3110_MAG3110_H
