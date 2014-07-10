#ifndef M3_UART_RX_H
#define M3_UART_RX_H

/* FRAME */
#define SYNC_BYTE 0x80
#define FRAME_LENGTH_MAX 32

typedef union {
    struct {
        uint8_t sync;
        uint8_t len;
        uint8_t type;
        uint8_t payload[FRAME_LENGTH_MAX - 3];
    };
    uint8_t data[FRAME_LENGTH_MAX];
} xRXFrame_t;


// type byte
enum frameType {
    LED_ON = 0,
    LED_OFF = 1,
    LED_BLINK = 2,
    GET_LIGHT = 3,
    GET_PRESSURE = 4,
    GET_GYRO = 5,
    GET_ACC = 6,
    GET_MAG = 7,
    TST_FLASH = 8,
    TST_I2C_CN = 9,
    TST_GPIO_PPS_START = 0xA,
    TST_GPIO_PPS_STOP = 0x1A,
    TST_GPIO_PPS_GET = 0x2A,
    RADIO_PKT = 0xB,
    GET_TIME = 0xD,
    GET_UID = 0xE,

    RADIO_PINGPONG = 0xbb,
    GPIO = 0xBE,
};


#endif // M3_UART_RX_H
