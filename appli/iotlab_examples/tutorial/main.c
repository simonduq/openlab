#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include <string.h>

#include "phy.h"
#include "soft_timer.h"
#include "event.h"

#include "lps331ap.h"
#include "isl29020.h"
#include "mac_csma.h"
#include "phy.h"

// choose channel int [1-16]
#define CHANNEL 10
#define ADDR_BROADCAST 0xFFFF

// UART callback function
static void char_rx(handler_arg_t arg, uint8_t c);

// timer alarm function
static void alarm(handler_arg_t arg);
static soft_timer_t tx_timer;
#define TX_PERIOD soft_timer_s_to_ticks(1)

// application task
static void app_task(void *);

/* Global variables */
// value storing the character received from the UART, analyzed by the main function
// volatile is required to prevent optimizations on it.
volatile int8_t cmd = 0;
// print help every second
volatile int8_t print_help = 1;

enum {
    NO_EVENT = 0,
    RX_PKT,
    TX_PKT,
    TX_PKT_ERROR,
};

// Got a radio event
volatile struct {
    int8_t got_event;

    uint8_t packet[256];
    uint16_t length;
    uint16_t addr;
    int16_t rssi;
} radio = {0};

/**
 * Sensors
 */
static void temperature_sensor()
{
    int16_t value;

    lps331ap_read_temp(&value);
    printf("Temperature measure: %f\n", 42.5 + value / 480.0);
}

static void light_sensor()
{
    float value = isl29020_read_sample();

    printf("Luminosity measure: %f lux\n", value);
}

static void pressure_sensor()
{
    uint32_t value;

    lps331ap_read_pres(&value);
    printf("Pressure measure: %f\n", value / 4096.0);
}


/*
 * Radio config
 */
static void send_packet()
{
    uint16_t ret;
    static uint8_t num = 0;

    // max pkt length <= max(cc2420, cc1101)
    snprintf((char*)&radio.packet, 58, "Hello World!: %u", num);
    radio.length = 1 + strlen((char*)&radio.packet);
    radio.addr = ADDR_BROADCAST;

    ret = mac_csma_data_send(radio.addr,(uint8_t *)&radio.packet, radio.length);
    num++;

    if (ret) {
        printf("mac_send ret %u\n", ret);
        radio.got_event = TX_PKT;
    } else {
        radio.got_event = TX_PKT_ERROR;
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
/* Reception of a radio message */
void mac_csma_data_indication(uint16_t src_addr,
        const uint8_t *data, uint8_t length, int8_t rssi, uint8_t lqi)
{
    radio.got_event = RX_PKT;

    strcpy((char*)radio.packet, (const char*)data);
    radio.length = length;
    radio.addr = src_addr;
    radio.rssi = rssi;

    printf("MYRSSI %d\n",  mac_csma_config.phy);

}
#pragma GCC diagnostic pop

/*
 * HELP
 */
static void print_usage()
{
    printf("\n\nIoT-LAB Simple Demo program\n");
    printf("Type command\n");
    printf("\th:\tprint this help\n");
    printf("\tt:\ttemperature measure\n");
    printf("\tl:\tluminosity measure\n");
    printf("\tp:\tpressure measure\n");
    printf("\ts:\tsend a radio packet\n");
    if (print_help)
        printf("\n Type Enter to stop printing this help\n");
    printf("\n");
}

static void hardware_init()
{
    // Openlab platform init
    platform_init();
    event_init();
    soft_timer_init();

    // Switch off the LEDs
    leds_off(LED_0);
    leds_off(LED_1);
    leds_off(LED_2);

    // Uart initialisation
    uart_set_rx_handler(uart_print, char_rx, NULL);

    // ISL29020 light sensor initialisation
    isl29020_prepare(ISL29020_LIGHT__AMBIENT, ISL29020_RESOLUTION__16bit,
            ISL29020_RANGE__16000lux);
    isl29020_sample_continuous();

    // LPS331AP pressure sensor initialisation
    lps331ap_powerdown();
    lps331ap_set_datarate(LPS331AP_P_12_5HZ_T_12_5HZ);

    // Init csma Radio mac layer
    mac_csma_init(CHANNEL);

    // Initialize a openlab timer
    soft_timer_set_handler(&tx_timer, alarm, NULL);
    soft_timer_start(&tx_timer, TX_PERIOD, 1);

}

static void handle_cmd(handler_arg_t arg)
{

    switch ((char) (uint32_t) arg) {
        case 't':
            temperature_sensor();
            break;
        case 'l':
            light_sensor();
            break;
        case 'p':
            pressure_sensor();
            break;
        case 's':
            send_packet();
            break;
        case '\n':
            break;
        case 'h':
        default:
            print_usage();
            break;
    }
    printf("cmd > ");
}

static void handle_radio()
{

    if (radio.got_event == NO_EVENT)
        return;

    printf("\nradio > ");

    switch (radio.got_event) {
        case RX_PKT:
            printf("Got packet from %x. Len: %u Rssi: %d: '%s'\n",
                    radio.addr, radio.length, radio.rssi, (char*)&radio.packet);
            break;
        case TX_PKT:
            printf("Packet sent\n");
            break;
        case TX_PKT_ERROR:
            printf("Packet sent failed\n");
            break;
        default:
            printf("Uknown event\n");
            break;
    }

}


int main()
{
    hardware_init();

    // Create a task for the application
    xTaskCreate(app_task, (const signed char * const) "app_task",
            configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    platform_run();

    return 0;
}


/* Reception of a char on UART and store it in 'cmd' */
static void char_rx(handler_arg_t arg, uint8_t c) {
    // disable help message after receiving char
    print_help = 0;


    if (c=='t' || c=='l' || c=='h' || c=='p' || c=='s' || c=='\n') {
        // copy received character to cmd variable.
        cmd = c;
        //handle_cmd(cmd);
        event_post_from_isr(EVENT_QUEUE_APPLI, handle_cmd,
                (handler_arg_t) (uint32_t) c);

    }
}


static void app_task(void *param)
{
    while (1) {

        while ((cmd == 0) && (radio.got_event == 0))
            soft_timer_delay_ms(10);

        if (cmd) {
            //handle_cmd(cmd);
            event_post(EVENT_QUEUE_APPLI, handle_cmd,
                    (handler_arg_t) (uint32_t) cmd);
            cmd = 0;
        }
        if (radio.got_event) {
            // disable help message
            print_help = 0;

            handle_radio();
            radio.got_event = 0;

            cmd = '\n';
        }
    }
}

static void alarm(handler_arg_t arg) {
    leds_toggle(LED_0);
    leds_toggle(LED_1);
    leds_toggle(LED_2);

    /* Print help before getting first real \n */
    if (print_help) {
        cmd='h';
    }

}
