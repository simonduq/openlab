#include "platform.h"

#include "iotlab_serial.h"
#include "iotlab_time.h"
#include "iotlab_leds.h"

#include "fiteco_lib_gwt.h"

#include "constants.h"
#include "cn_control.h"

/* for flush in reset_time */
#include "cn_consumption.h"
#include "cn_radio.h"



static int32_t on_start(uint8_t cmd_type, packet_t *pkt);
static int32_t on_stop(uint8_t cmd_type, packet_t *pkt);

static int32_t reset_time(uint8_t cmd_type, packet_t *pkt);
static void do_reset_time(handler_arg_t arg);

static soft_timer_t green_led_alarm;
static int32_t green_led_blink(uint8_t cmd_type, packet_t *pkt);
static int32_t green_led_on(uint8_t cmd_type, packet_t *pkt);


void cn_control_start()
{
    // Configure and register all handlers
    static iotlab_serial_handler_t handler_start;
    static iotlab_serial_handler_t handler_stop;

    handler_start.cmd_type = OPEN_NODE_START;
    handler_start.handler = on_start;
    iotlab_serial_register_handler(&handler_start);

    handler_stop.cmd_type = OPEN_NODE_STOP;
    handler_stop.handler = on_stop;
    iotlab_serial_register_handler(&handler_stop);


    // reset_time
    static iotlab_serial_handler_t handler_reset_time;
    handler_reset_time.cmd_type = RESET_TIME;
    handler_reset_time.handler = reset_time;
    iotlab_serial_register_handler(&handler_reset_time);


    // green led control
    static iotlab_serial_handler_t handler_green_led_blink;
    static iotlab_serial_handler_t handler_green_led_on;

    handler_green_led_blink.cmd_type = GREEN_LED_BLINK;
    handler_green_led_blink.handler = green_led_blink;
    iotlab_serial_register_handler(&handler_green_led_blink);

    handler_green_led_on.cmd_type = GREEN_LED_ON;
    handler_green_led_on.handler = green_led_on;
    iotlab_serial_register_handler(&handler_green_led_on);
}

static int32_t on_start(uint8_t cmd_type, packet_t *pkt)
{
    if (1 != pkt->length)
        return 1;

    // DC <=> charge
    if (DC == *pkt->data) {
        fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__MAIN);
        fiteco_lib_gwt_battery_charge_enable();
    } else if (BATTERY == *pkt->data) {
        fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__BATTERY);
        fiteco_lib_gwt_battery_charge_disable();
    } else {
        //unexpected value
        return 1;
    }

    return 0;
}

static int32_t on_stop(uint8_t cmd_type, packet_t *pkt)
{
    if (1 != pkt->length)
        return 1;

    fiteco_lib_gwt_opennode_power_select(FITECO_GWT_OPENNODE_POWER__OFF);

    // Charge if DC
    if (DC == *pkt->data)
        fiteco_lib_gwt_battery_charge_enable();
    else if (BATTERY == *pkt->data)
        fiteco_lib_gwt_battery_charge_disable();
    else
        return 1;

    return 0;
}

int32_t reset_time(uint8_t cmd_type, packet_t *pkt)
{
    if (0 != pkt->length)
        return 1;

    /* alloc the ack frame */
    packet_t *ack_pkt = iotlab_serial_packet_alloc();
    if (!ack_pkt)
        return 1;
    ack_pkt->data[0] = RESET_TIME;
    ack_pkt->length = 1;

    if (event_post(EVENT_QUEUE_APPLI, do_reset_time, ack_pkt))
        return 1;

    return 0;
}

static void do_reset_time(handler_arg_t arg)
{
    packet_t *pkt = (packet_t *)arg;

    /*
     * Flush measures packets
     */
    flush_current_consumption_measures();
    flush_current_rssi_measures();


    /* Send the update frame */
    if (iotlab_serial_send_frame(ACK_FRAME, pkt)) {
        // ERF that's really bad, config failed
        // send ERROR NOW TODO
        packet_free(pkt);
        return;
    }

    iotlab_time_reset_time();
}

/*
 * green led control
 */

int32_t green_led_blink(uint8_t cmd_type, packet_t *pkt)
{
    leds_blink(&green_led_alarm, soft_timer_s_to_ticks(1), GREEN_LED);
    return 0;
}

int32_t green_led_on(uint8_t cmd_type, packet_t *pkt)
{
    leds_blink(&green_led_alarm, 0, GREEN_LED); // stop
    leds_on(GREEN_LED);

    return 0;
}

