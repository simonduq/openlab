/*
 * In the build directory just under the openlab directory:
 * cmake .. -DRELEASE=2 -DPLATFORM=iotlab-m3
 * cmake .. -DRELEASE=2 -DPLATFORM=iotlab-a8-m3
 * -DRELEASE=2 to avoid any log_printf
 */
#include <stdint.h>
#include <string.h>
#include "printf.h"

#include "platform.h"

/* Drivers */
#include "unique_id.h"
#include "l3g4200d.h"
#include "lsm303dlhc.h"
#ifdef IOTLAB_M3
#include "lps331ap.h"
#include "isl29020.h"
#include "n25xxx.h"
#endif

#include "uart_rx.h"
#include "iotlab_gpio.h"
#include "iotlab_autotest_on.h"


// L3G4200D ST Sensitivity specification page 10/42
#define GYR_SENS_8_75    (8.75e-3)  // for ±250dps   scale in dps/digit
// LSM303DLH ST Sensitivity specification page 11/47
#define ACC_SENS_2G      (1e-3)     // for ±2g       scale in g/digit
#define MAG_SENS_1_3_XY  (1/1055.)  // for ±1.3gauss scale in gauss/LSB
#define MAG_SENS_1_3_Z   (1/950.)   // for ±1.3gauss scale in gauss/LSB

#define ON_ERROR(msg, label)  do { \
    printf("NACK %s\n", (msg)); \
    goto label; \
} while (0)
#define ONE_SECOND  portTICK_RATE_MS * 1000

/* Local variables */
static phy_packet_t tx_pkt;
static xQueueHandle radio_queue;

static xRXFrame_t xRxFrame;


/* Function Protypes */
/*
 * UART interrupt Handler
 * Recognize command frame received from the A8
 * If so put an event in the high priority queue with vRX_manager as handler
 * \param arg the argument to pass to the handler function, here it is NULL
 * \param c the character received by the UART
 */
static void char_handler_irq(handler_arg_t arg, uint8_t c);

/*
 * Parse the command frame, execute the command
 * And from these operations results, prepare the response frame
 */
static void vParseFrame();


#ifdef IOTLAB_A8_M3
static volatile uint32_t seconds = 0;

static void pps_handler_irq(handler_arg_t arg)
{
    (void)arg;
    seconds++;
}
#endif

static void radio_rx_tx_done(phy_status_t status)
{
    int success = (PHY_SUCCESS == status);
    xQueueSend(radio_queue, &success, 0);
}


static void send_radio_pkt(uint8_t tx_power, uint8_t channel)
{
    int success;
    xQueueReceive(radio_queue, &success, 0);  // cleanup queue

    phy_idle(platform_phy);

    /*
     * config radio
     */
    if (phy_set_channel(platform_phy, channel))
        ON_ERROR("RADIO_PKT_ERR_SET_CHANNEL", radio_cleanup);
    if (phy_set_power(platform_phy, tx_power))
        ON_ERROR("RADIO_PKT_ERR_SET_POWER", radio_cleanup);

    /*
     * Send packet
     *
     * No interlocking because
     *     Current queue is EVENT_QUEUE_APPLI
     *     phy_ handlers are executed by EVENT_QUEUE_NETWORK
     */
    phy_packet_t tx_pkt_noise;
    tx_pkt_noise.length = 125;
    if (phy_tx_now(platform_phy, &tx_pkt_noise, radio_rx_tx_done))
        ON_ERROR("RADIO_PKT_ERR_TX", radio_cleanup);
    if (pdTRUE != xQueueReceive(radio_queue, &success, ONE_SECOND) || !success)
        ON_ERROR("RADIO_PKT_ERR_TX_DONE", radio_cleanup);

    printf("ACK RADIO_PKT\n");

radio_cleanup:
    phy_reset(platform_phy);
}

static void radio_ping_pong(uint8_t tx_power, uint8_t channel)
{
    int success;
    phy_packet_t rx_pkt;
    xQueueReceive(radio_queue, &success, 0);  // cleanup queue

    phy_idle(platform_phy);

    /* config radio */
    if (phy_set_channel(platform_phy, channel))
        ON_ERROR("RADIO_PINGPONG_ERR_SET_CHANNEL", ping_pong_cleanup);
    if (phy_set_power(platform_phy, tx_power))
        ON_ERROR("RADIO_PINGPONG_ERR_SET_POWER", ping_pong_cleanup);

    /*
     * Send packet
     *
     * No interlocking because
     *     Current queue is EVENT_QUEUE_APPLI
     *     phy_ handlers are executed by EVENT_QUEUE_NETWORK
     */
    if (phy_tx_now(platform_phy, &tx_pkt, radio_rx_tx_done))
        ON_ERROR("RADIO_PINGPONG_ERR_PHY_TX", ping_pong_cleanup);
    if (pdTRUE != xQueueReceive(radio_queue, &success, ONE_SECOND) || !success)
        ON_ERROR("RADIO_PINGPONG_ERR_TX_FAILED", ping_pong_cleanup);

    /*
     * Wait for answer
     */
    phy_prepare_packet(&rx_pkt);
    phy_rx(platform_phy, 0, 0, &rx_pkt, radio_rx_tx_done);
    if (pdTRUE != xQueueReceive(radio_queue, &success, ONE_SECOND) || !success)
        ON_ERROR("RADIO_PINGPONG_RX_TIMEOUT", ping_pong_cleanup);

    // Packet reception
    if (0 >= rx_pkt.length) {
        printf("NACK RADIO_PINGPONG_PACKET_LENGTH_<=0 length=%d\n");
    } else if (0 == strcmp("RX_PKT_HELLO_WORLD", (char *)rx_pkt.raw_data)) {
        printf("ACK RADIO_PINGPONG\n");
    } else {
        printf("NACK RADIO_PINGPONG_WRONG_PACKET_RECEIVED\n");
    }

ping_pong_cleanup:
    phy_reset(platform_phy);
}

#ifdef IOTLAB_M3
static int test_flash_nand()
{
    static uint8_t buf_EE[256] = {[0 ... 255] = 0xEE};
    static uint8_t buf[256]    = {[0 ... 255] = 0x00};

    // Write subsector 200 and re-read it
    n25xxx_write_enable(); n25xxx_erase_subsector(0x00c80000);
    n25xxx_write_enable(); n25xxx_write_page(0x00c80000, buf_EE);
    n25xxx_read(0x00c80000, buf, sizeof(buf));
    n25xxx_write_enable(); n25xxx_erase_subsector(0x00c80000);

    // check read values
    return memcmp(buf_EE, buf, sizeof(buf));
}
#endif // IOTLAB_M3


/**
 * The main function.
 * Initialize the hardware, libraries uses and variables.
 * Launch the freeRTOS scheduler
 */

int main(void) {
    /* Setup the hardware. */
    platform_init();

    /* setup UART1 handler which is connected to the gw handler */
    uart_set_rx_handler(uart_print, char_handler_irq, NULL);

    radio_queue = xQueueCreate(1, sizeof(int));  // radio sync queue

    //init the sw timer
    soft_timer_init();

    //init sensor
#ifdef IOTLAB_M3
    isl29020_prepare(ISL29020_LIGHT__AMBIENT, ISL29020_RESOLUTION__16bit,
            ISL29020_RANGE__1000lux);
    isl29020_sample_continuous();

    lps331ap_powerdown();
    lps331ap_set_datarate(LPS331AP_P_12_5HZ_T_12_5HZ);
#endif

    l3g4200d_powerdown();
    l3g4200d_gyr_config(L3G4200D_200HZ, L3G4200D_250DPS, true);

    lsm303dlhc_powerdown();
    lsm303dlhc_mag_config(
            LSM303DLHC_MAG_RATE_220HZ, LSM303DLHC_MAG_SCALE_2_5GAUSS,
            LSM303DLHC_MAG_MODE_CONTINUOUS, LSM303DLHC_TEMP_MODE_ON);
    lsm303dlhc_acc_config(
            LSM303DLHC_ACC_RATE_400HZ, LSM303DLHC_ACC_SCALE_2G,
            LSM303DLHC_ACC_UPDATE_ON_READ);

    //prepare radio TX packet
    char msg[] = "TX_PKT_HELLO_WORLD";
    tx_pkt.data = tx_pkt.raw_data;
    tx_pkt.length = sizeof(msg);
    memcpy(tx_pkt.data, msg, tx_pkt.length);


    platform_run();
    printf("FATAL ERROR\n");
    return 1;
}

void led_blink(handler_arg_t leds_config)
{
    leds_toggle((uint8_t) (uint32_t) leds_config);
}


static void vParseFrame()
{
    uint32_t led_period;
    int16_t xyz[3];
    char *i2c2_err_msg;
    static soft_timer_t led_alarm;

#ifdef IOTLAB_M3
    uint32_t pressure;
#endif

    /* Analyse the Command, and take corresponding action. */
    switch (xRxFrame.type) {

    case LED_ON:
        // 7 for all, 4 for 3rd, 2 for 2nd, 1 for 1st
        leds_on(xRxFrame.payload[0]);
        printf("ACK LEDS_ON config=%x\n", xRxFrame.payload[0]);
        break;

    case LED_OFF:
        leds_off(xRxFrame.payload[0]);
        printf("ACK LEDS_OFF config=%x\n", xRxFrame.payload[0]);
        break;

    case LED_BLINK:

        //time in ms from 0 to 65535ms for half a period of blink
        led_period  = xRxFrame.payload[1];
        led_period |= xRxFrame.payload[2] << 8;
        handler_arg_t arg = (handler_arg_t) (uint32_t) xRxFrame.payload[0];

        if (led_period) {
            soft_timer_set_handler(&led_alarm, led_blink, arg);
            soft_timer_start(&led_alarm, soft_timer_ms_to_ticks(led_period), 1);

            printf("ACK LED_BLINK config=%x period=%d\n",
                    xRxFrame.payload[0], led_period);
        } else {
            soft_timer_stop(&led_alarm);
            printf("ACK LED_BLINK STOP\n");
        }
        break;


#ifdef IOTLAB_M3

    case GET_LIGHT:
        printf("ACK LIGHT = %f lux\n", isl29020_read_sample());
        break;

    case GET_PRESSURE:
        lps331ap_read_pres(&pressure);
        printf("ACK PRESSURE = %f mbar\n", pressure / 4096.0);
        break;

    case TST_FLASH:
        if (test_flash_nand())
            printf("NACK ERR_COMPARING_WRITE_&_READ\n");
        else
            printf("ACK TST_FLASH\n");
        break;

#endif


    case GET_GYRO:
        if (l3g4200d_read_rot_speed(xyz))
            printf("NACK GYRO_READ_ERR\n");
        else
            printf("ACK GYRO_ROTATION_SPEED = %f %f %f dps\n",
                    xyz[0] * GYR_SENS_8_75,
                    xyz[1] * GYR_SENS_8_75,
                    xyz[2] * GYR_SENS_8_75);
        break;


    case GET_ACC:
        if (lsm303dlhc_read_acc(xyz))
            printf("NACK ACCELERO_READ_ERR\n");
        else
            printf("ACK ACCELERATION = %f %f %f\n",
                    xyz[0] * ACC_SENS_2G,
                    xyz[1] * ACC_SENS_2G,
                    xyz[2] * ACC_SENS_2G);
        break;

    case GET_MAG:
        if (lsm303dlhc_read_mag(xyz))
            printf("NACK MAGNETO_READ_ERROR\n");
        else
            printf("ACK MAGNETO = %f %f %f\n",
                    xyz[0] * MAG_SENS_1_3_XY,
                    xyz[1] * MAG_SENS_1_3_XY,
                    xyz[2] * MAG_SENS_1_3_Z);
        break;


    case TST_I2C_CN:
        i2c2_err_msg = on_test_i2c2();
        if (NULL == i2c2_err_msg)
            printf("ACK I2C2_CN\n");
        else
            printf("NACK %s\n", i2c2_err_msg);
        break;

#ifdef IOTLAB_A8_M3

    case TST_GPIO_PPS_START:

        seconds = 0;
        // third gpio line
        gpio_enable_irq(&gpio_config[3], IRQ_RISING, pps_handler_irq, NULL);
        printf("ACK GPS_PPS_START\n");
        break;

    case TST_GPIO_PPS_STOP:
        gpio_disable_irq(&gpio_config[3]);
        printf("ACK GPS_PPS_STOP = %d pps\n", seconds);
        break;

#endif

    case GET_TIME:
        printf("ACK CURRENT_TIME = %d tick_32khz\n", soft_timer_time());
        break;

    case RADIO_PKT:
        send_radio_pkt(xRxFrame.payload[0], xRxFrame.payload[1]);
        break;

    case RADIO_PINGPONG:
        radio_ping_pong(xRxFrame.payload[0], xRxFrame.payload[1]);
        break;

    case GPIO:
        if (on_test_gpio())
            printf("NACK GPIO\n");
        else
            printf("ACK GPIO\n");
        break;

    case GET_UID:
        printf("ACK UID = %08x%08x%08x\n", uid->uid32[0],
                uid->uid32[1], uid->uid32[2]);
        break;

    default:
        printf("NACK ERROR_DEFENSIVE_UNKNOWN_COMMAND\n");
        break;

    }
}

static void char_handler_irq(handler_arg_t arg, uint8_t c)
{
    static uint16_t ix = 0;

    if (ix == 0) {
        if (c == SYNC_BYTE)
            xRxFrame.data[ix++] = c;  // First byte, SYNC
    } else if (ix == 1) {
        if ((0 < c) && (c <= FRAME_LENGTH_MAX))
            xRxFrame.data[ix++] = c;  // Valid length byte
        else
            ix = 0;  // Reset frame
    } else if (ix == (uint16_t) (xRxFrame.len + 1)) {
        //end of frame
        xRxFrame.data[ix] = c;
        /*
         * The queue is EVENT_QUEUE_APPLI. For the pingpong/RADIO_PKT test
         * Setting it to EVENT_QUEUE_NETWORK break their functionality.
         * PHY layer uses the EVENT_QUEUE_NETWORK. Ex: pinpong do a radio rx
         *  then if the queue used here is the same, PHY layer will post event
         *  to unlock the semaphore on the same queue so the same task.
         *  But this task will be blocked on semaphore so never ending story.
         */
        if (EVENT_FULL == event_post_from_isr(EVENT_QUEUE_APPLI,
                (handler_t) vParseFrame, NULL))
            printf("NACK APPLI_QUEUE_OVERFLOW\n");
        ix = 0;
    } else {
        xRxFrame.data[ix++] = c;
    }
}
