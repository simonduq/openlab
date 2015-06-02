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
 * sensor.c
 *
 * \brief IMU sensor
 *
 * \date Jan 06, 2015
 * \author: <roger.pissard.at.inria.fr>
 */

#include "platform.h"
#include "debug.h"

#include "lsm303dlhc.h"
#include "l3g4200d.h"
#include "sensor.h"

/* CONFIG.TXT
mag_freq = 15
mag_scale = 2.5
gyr_freq = 400
gyr_scale = 500
acc_freq = 400
acc_scale = 8
 */

void init_sensor()
{
  /* Configure accelero */ 
  lsm303dlhc_powerdown();
  lsm303dlhc_acc_config(LSM303DLHC_ACC_RATE_400HZ,
			LSM303DLHC_ACC_SCALE_8G,
			LSM303DLHC_ACC_UPDATE_ON_READ);
  
  /* Configure magneto */
  lsm303dlhc_mag_config(LSM303DLHC_MAG_RATE_220HZ,
			LSM303DLHC_MAG_SCALE_2_5GAUSS, 
			LSM303DLHC_MAG_MODE_CONTINUOUS,
			LSM303DLHC_TEMP_MODE_ON);
  /* Configure gyro */
  l3g4200d_gyr_config(L3G4200D_400HZ, L3G4200D_500DPS, true);
}

void read_sensor(imu_sensor_data_t *imu)
{
  /* Read accelero */
  lsm303dlhc_read_acc(imu->acc); 
  /* Read magneto */
  lsm303dlhc_read_mag(imu->mag);
  /* Read gyro */
  l3g4200d_read_rot_speed(imu->gyr);
}
