#ifndef IOTLAB_I2C_COMMON_H
#define IOTLAB_I2C_COMMON_H

enum {
    IOTLAB_I2C_CN_ADDR = 0x42,
    IOTLAB_I2C_ERROR   = 0xFF,
};

enum {
    IOTLAB_I2C_RX_TIME    = 0x00,   // len 8
    IOTLAB_I2C_RX_NODE_ID = 0x01,   // len 2

    IOTLAB_I2C_TX_EVENT   = 0xF0,  // len 2
};

#endif // IOTLAB_I2C_COMMON_H
