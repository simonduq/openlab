#ifndef IOTLAB_I2C_COMMON_H
#define IOTLAB_I2C_COMMON_H

enum {
    IOTLAB_I2C_CN_ADDR = 0x42,
    IOTLAB_I2C_ERROR   = 0xEE,
};
struct timeval {
    uint32_t t_s;
    uint32_t t_us;
};

typedef enum {
    TX,
    RX,
} i2c_msg_type_t;

struct iotlab_i2c_handler_arg {
    uint8_t payload[32];
    size_t len;
    struct timeval timestamp;
};

struct iotlab_i2c_handler {

    uint8_t header;
    i2c_msg_type_t type;
    uint8_t payload_len;
    handler_t handler;

    struct iotlab_i2c_handler *next;
};

#endif // IOTLAB_I2C_COMMON_H
