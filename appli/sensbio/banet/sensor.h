/*
* This file is a part of openlab/sensbiotk
*
* Copyright (C) 2015  INRIA (Contact: sensbiotk@inria.fr)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * sensor.h
 *
 * \brief sensor data definition
 *
 * \date Jan 06, 2015
 * \author: <roger.pissard.at.inria.fr>
 */

#ifndef _SENSOR_H
#define  _SENSOR_H

typedef struct {
  uint16_t seq;
  int16_t acc[3];
  int16_t mag[3];
  int16_t gyr[3];
} imu_sensor_data_t;

extern void init_sensor();

extern void read_sensor(imu_sensor_data_t *imu);

#endif /* _SENSOR_H */
