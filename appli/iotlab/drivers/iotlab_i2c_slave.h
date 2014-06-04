#ifndef IOTLAB_I2C_SLAVE_H
#define IOTLAB_I2C_SLAVE_H

#include "handler.h"
#include "iotlab_i2c_common.h"


void iotlab_i2c_slave_register_handler(struct iotlab_i2c_handler *handler);

void iotlab_i2c_slave_start();

#endif // IOTLAB_I2C_SLAVE_H
