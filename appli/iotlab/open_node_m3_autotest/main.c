/*
 * In the build directory just under the openlab directory:
 * cmake .. -DRELEASE=2 -DPLATFORM=iotlab-m3
 * cmake .. -DRELEASE=2 -DPLATFORM=iotlab-a8-m3
 * -DRELEASE=2 to avoid any log_printf
 */
#include <stdint.h>
#include <string.h>
#include "printf.h"
#include "scanf.h"

#include "platform.h"
#include "shell.h"

/* Drivers */
#include "unique_id.h"
#include "l3g4200d.h"
#include "lsm303dlhc.h"
#ifdef IOTLAB_M3
#include "lps331ap.h"
#include "isl29020.h"
#include "n25xxx.h"
#endif

#include "phy_power.c.h"

#include "iotlab_gpio.h"
#include "iotlab_autotest_on.h"

#define COMMAND_LEN 256



#define ON_ERROR(msg, label)  do { \
    ret = msg; \
    goto label; \
} while (0)

/* Local variables */
static xQueueHandle radio_queue;

#define RADIO_TX_STR  "TX_PKT_HELLO_WORLD"

// L3G4200D ST Sensitivity specification page 10/42
#define GYR_SENS_8_75    (8.75e-3)  // for ±250dps   scale in dps/digit
// LSM303DLH ST Sensitivity specification page 11/47
#define ACC_SENS_2G      (1e-3)     // for ±2g       scale in g/digit
#define MAG_SENS_1_3_XY  (1/1055.)  // for ±1.3gauss scale in gauss/LSB
#define MAG_SENS_1_3_Z   (1/950.)   // for ±1.3gauss scale in gauss/LSB
#define ONE_SECOND  portTICK_RATE_MS * 1000

/* Simple Commands */
static int cmd_get_time(int argc, char **argv)
{
    if (argc != 1)
        return 1;
    printf("ACK %s %u ticks_32khz\n", argv[0], soft_timer_time());
    return 0;
}

static int cmd_get_uid(int argc, char **argv)
{
    if (argc != 1)
        return 1;
    printf("ACK %s %08x%08x%08x\n", argv[0],
            uid->uid32[0], uid->uid32[1], uid->uid32[2]);
    return 0;
}

static int cmd_echo(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++)
        if (i == 1)
            printf("%s", argv[i]);
        else
            printf(" %s", argv[i]);
    printf("\n");
    return 0;
}

/* Leds Commands */
static int cmd_leds_on(int argc, char **argv)
{
    uint8_t leds = 0;
    if (argc != 2)
        return 1;

    if (1 != sscanf(argv[1], "%u", &leds))
        return 1;

    leds_on(leds);
    printf("ACK %s %x\n", argv[0], leds);
    return 0;
}

static int cmd_leds_off(int argc, char **argv)
{
    uint8_t leds = 0;
    if (argc != 2)
        return 1;

    if (!sscanf(argv[1], "%u", &leds))
        return 1;

    leds_off(leds);
    printf("ACK %s %x\n", argv[0], leds);
    return 0;
}


static int cmd_leds_blink(int argc, char **argv)
{
    uint8_t leds = 0;
    uint32_t time = 0;
    static soft_timer_t led_alarm;

    if (argc != 3)
        return 1;
    if (!sscanf(argv[1], "%u", &leds))
        return 1;
    if (!sscanf(argv[2], "%u", &time))
        return 1;

    if (time) {
        soft_timer_set_handler(&led_alarm, (handler_t)leds_toggle,
                (handler_arg_t)(uint32_t)leds);
        soft_timer_start(&led_alarm, soft_timer_ms_to_ticks(time), 1);
        printf("ACK %s %u %u\n", argv[0], leds, time);
    } else {
        soft_timer_stop(&led_alarm);
        printf("ACK %s stop\n", argv[0]);
    }
    return 0;
}

static void radio_rx_tx_done(phy_status_t status)
{
    int success = (PHY_SUCCESS == status);
    xQueueSend(radio_queue, &success, 0);
}


static char *radio_pkt(uint8_t channel, uint8_t tx_power)
{
    static phy_packet_t tx_pkt = {
        .data = tx_pkt.raw_data,
        .length = 125,
    };
    uint16_t i;
    for (i = 0; i < 125; i++) {
        tx_pkt.data[i] = i;
    }

    int success;
    char *ret = NULL;
    xQueueReceive(radio_queue, &success, 0);  // cleanup queue

    phy_idle(platform_phy);

    /*
     * config radio
     */
    if (phy_set_channel(platform_phy, channel))
        ON_ERROR("err_set_channel", radio_cleanup);
    if (phy_set_power(platform_phy, tx_power))
        ON_ERROR("err_set_power", radio_cleanup);

    /*
     * Send packet
     *
     * No interlocking because
     *     Current queue is EVENT_QUEUE_APPLI
     *     phy_ handlers are executed by EVENT_QUEUE_NETWORK
     */
    if (phy_tx_now(platform_phy, &tx_pkt, radio_rx_tx_done))
        ON_ERROR("err_tx", radio_cleanup);
    if (pdTRUE != xQueueReceive(radio_queue, &success, ONE_SECOND) || !success)
        ON_ERROR("tx_failed", radio_cleanup);

    // SUCCESS

radio_cleanup:
    phy_reset(platform_phy);
    return ret;
}

static char *radio_ping_pong(uint8_t channel, uint8_t tx_power)
{
    char *ret = NULL;
    int success;
    static phy_packet_t tx_pkt = {
        .data = tx_pkt.raw_data,
        .length = sizeof(RADIO_TX_STR),
        .raw_data = RADIO_TX_STR,
    };
    static phy_packet_t rx_pkt = {.data = rx_pkt.raw_data};

    xQueueReceive(radio_queue, &success, 0);  // cleanup queue

    phy_idle(platform_phy);

    /* config radio */
    if (phy_set_channel(platform_phy, channel))
        ON_ERROR("err_set_channel", ping_pong_cleanup);
    if (phy_set_power(platform_phy, tx_power))
        ON_ERROR("err_set_power", ping_pong_cleanup);

    /*
     * Send packet
     *
     * No interlocking because
     *     Current queue is EVENT_QUEUE_APPLI
     *     phy_ handlers are executed by EVENT_QUEUE_NETWORK
     */
    if (phy_tx_now(platform_phy, &tx_pkt, radio_rx_tx_done))
        ON_ERROR("err_tx", ping_pong_cleanup);
    if (pdTRUE != xQueueReceive(radio_queue, &success, ONE_SECOND) || !success)
        ON_ERROR("tx_failed", ping_pong_cleanup);

    /*
     * Wait for answer
     */
    memset(rx_pkt.raw_data, 0, sizeof(rx_pkt.raw_data));
    phy_rx_now(platform_phy, &rx_pkt, radio_rx_tx_done);
    if (pdTRUE != xQueueReceive(radio_queue, &success, ONE_SECOND) || !success)
        ON_ERROR("rx_timeout", ping_pong_cleanup);

    // Packet reception
    if (strcmp("RX_PKT_HELLO_WORLD", (char *)rx_pkt.raw_data))
        ON_ERROR("wrong_packet_received", ping_pong_cleanup);

    // SUCCESS
    ret = NULL;

ping_pong_cleanup:
    phy_reset(platform_phy);
    return ret;
}

#ifdef IOTLAB_M3

////////
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
////////


static int cmd_get_light(int argc, char **argv)
{
    if (argc != 1)
        return 1;
    printf("ACK %s %f lux\n", argv[0], isl29020_read_sample());
    return 0;
}

static int cmd_get_pressure(int argc, char **argv)
{
    if (argc != 1)
        return 1;

    uint32_t pressure;
    lps331ap_read_pres(&pressure);
    printf("ACK %s %f mbar\n", argv[0], pressure / 4096.0);
    return 0;
}

static int cmd_test_flash_nand(int argc, char **argv)
{
    if (argc != 1)
        return 1;

    if (test_flash_nand())
        printf("NACK %s read_different_write\n", argv[0]);
    else
        printf("ACK %s \n", argv[0]);
    return 0;
}

#endif // IOTLAB_M3



#if defined(IOTLAB_M3) || defined(IOTLAB_A8_M3)
/* ON<->CN Commands */
static int cmd_test_i2c(int argc, char **argv)
{
    if (argc != 1)
        return 1;

    char *i2c2_err_msg = on_test_i2c2();
    if (NULL == i2c2_err_msg)
        printf("ACK %s\n", argv[0]);
    else
        printf("NACK %s %s\n", argv[0], i2c2_err_msg);
    return 0;
}

static int cmd_test_gpio(int argc, char **argv)
{
    if (argc != 1)
        return 1;

    if (on_test_gpio())
        printf("NACK %s\n", argv[0]);
    else
        printf("ACK %s\n", argv[0]);
    return 0;
}
#endif

/* /ON<->CN Commands */



/* Get Sensor */

static int _cmd_get_xyz(int argc, char **argv, char *unit,
        unsigned int (*sensor)(int16_t*),
        float x_factor, float y_factor, float z_factor)
{
    if (argc != 1)
        return 1;

    int16_t xyz[3];
    if (sensor(xyz))
        printf("NACK %s error\n", argv[0]);
    else
        printf("ACK %s %f %f %f %s\n", argv[0],
                xyz[0] * x_factor, xyz[1] * y_factor, xyz[2] * z_factor,
                unit);
    return 0;
}


static int cmd_get_gyro(int argc, char **argv)
{
    return _cmd_get_xyz(argc, argv, "dps", l3g4200d_read_rot_speed,
            GYR_SENS_8_75, GYR_SENS_8_75, GYR_SENS_8_75);
}

static int cmd_get_accelero(int argc, char **argv)
{
    return _cmd_get_xyz(argc, argv, "g", lsm303dlhc_read_acc,
            ACC_SENS_2G, ACC_SENS_2G, ACC_SENS_2G);
}

static int cmd_get_magneto(int argc, char **argv)
{
    return _cmd_get_xyz(argc, argv, "gauss", lsm303dlhc_read_mag,
            MAG_SENS_1_3_XY, MAG_SENS_1_3_XY, MAG_SENS_1_3_Z);
}

/* /Get Sensor */

/* Radio */

static int _radio_cmd(int argc, char **argv,
        char *(*radio_fct)(uint8_t, uint8_t))
{
    if (argc != 3)
        return 1;

    // Channel
    uint8_t channel;
    if (1 != sscanf(argv[1], "%u", &channel))
        return 1;
    if (11 > channel || channel > 26)
        return 1;

    // Power
    uint8_t power;
    power = parse_power_rf231(argv[2]);
    if (255 == power) {
        printf("NACK power %s not in \n%s\n", argv[2], radio_power_rf231_str);
        return 1;
    }

    char *err_msg = radio_fct(channel, power);
    if (NULL == err_msg)
        printf("ACK %s %u %s\n", argv[0], channel, argv[2]);
    else
        printf("NACK %s %s\n", argv[0], err_msg);

    return 0;
}

static int cmd_radio_pkt(int argc, char **argv)
{
    return _radio_cmd(argc, argv, radio_pkt);
}

static int cmd_radio_ping_pong(int argc, char **argv)
{
    return _radio_cmd(argc, argv, radio_ping_pong);
}



#ifdef IOTLAB_A8_M3 // GPS

static volatile uint32_t seconds = 0;
static void pps_handler_irq(handler_arg_t arg)
{
    (void)arg;
    seconds++;
}

static int cmd_test_pps_start(int argc, char **argv)
{
    if (argc != 1)
        return 1;
    // third gpio line
    seconds = 0;
    gpio_enable_irq(&gpio_config[3], IRQ_RISING, pps_handler_irq, NULL);
    printf("ACK %s\n", argv[0]);
    return 0;
}

static int cmd_test_pps_stop(int argc, char **argv)
{
    if (argc != 1)
        return 1;
    // third gpio line
    seconds = 0;
    gpio_disable_irq(&gpio_config[3]);
    printf("ACK %s\n", argv[0]);
    return 0;
}

static int cmd_test_pps_get(int argc, char **argv)
{
    if (argc != 1)
        return 1;
    printf("ACK %s %d pps\n", argv[0], seconds);
    return 0;
}

#endif // A8 - GPS


struct shell_command commands[] = {
    {"echo",           "echo given arguments", cmd_echo},
    {"get_time",       "Print board time",     cmd_get_time},
    {"get_uid",        "Print board uid",      cmd_get_uid},

    {"leds_on",        "[leds_flag] Turn given leds on",  cmd_leds_on},
    {"leds_off",       "[leds_flag] Turn given leds off", cmd_leds_off},
    {"leds_blink",     "[leds_flag] [time] Blink leds every 'time'. If 'time' == 0 disable", cmd_leds_blink},

    {"get_accelero",   "Get accelero sensor",   cmd_get_accelero},
    {"get_magneto",    "Get magnneto sensor",   cmd_get_magneto},
    {"get_gyro",       "Get gyro sensor",       cmd_get_gyro},

#if defined(IOTLAB_M3) || defined(IOTLAB_A8_M3)
    {"test_i2c",       "Test i2c with CN",      cmd_test_i2c},
    {"test_gpio",      "Test gpio with CN",     cmd_test_gpio},
#endif//

#ifdef IOTLAB_M3
    {"get_light",      "Get light sensor",      cmd_get_light},
    {"get_pressure",   "Get pressure sensor",   cmd_get_pressure},
    {"test_flash",     "Test flash nand",       cmd_test_flash_nand},
#endif

    {"radio_pkt",       "[channel] [power] Send radio packet",    cmd_radio_pkt},
    {"radio_ping_pong", "[channel] [power] Radio ping pong test", cmd_radio_ping_pong},

#ifdef IOTLAB_A8_M3
    {"test_pps_start", "Start PPS test",        cmd_test_pps_start},
    {"test_pps_stop",  "Stop PPS test",         cmd_test_pps_stop},
    {"test_pps_get",   "Get current PPS count", cmd_test_pps_get},
#endif

    {NULL, NULL, NULL},
};

int main(void)
{
    platform_init();
    soft_timer_init();
    event_init();

    radio_queue = xQueueCreate(1, sizeof(int));  // radio sync queue

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

    shell_init(commands, 0);
    platform_run();
    return 1;
}

