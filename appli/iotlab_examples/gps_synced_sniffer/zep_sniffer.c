#include <string.h>
#include "platform.h"

#include "gps_synced_clock.h"
#include "zep_sniffer_format.h"

#define LOG_LEVEL LOG_LEVEL_INFO
#include "debug.h"

/* [CA] the following was copied and adapted from
   openlab/appli/iotlab_tests/foren6_sniffer/foren6_sniffer.c
   Itself was: */
/*
 *
 * Copied and adapted from:
 *     https://github.com/cetic/contiki/blob/sniffer/examples/sniffer/sniffer.c
 *
 */


// Dropped all the serial configuration of the channel




//
//   From packet-zep.c in wireshark:
//   ZEP v2 Header will have the following format (if type=1/Data):
//   |Preamble|Version| Type |Channel ID|Device ID|CRC/LQI Mode|LQI Val|NTP Timestamp|Sequence#|Reserved|Length|
//   |2 bytes |1 byte |1 byte|  1 byte  | 2 bytes |   1 byte   |1 byte |   8 bytes   | 4 bytes |10 bytes|1 byte|
//



#define MAGIC_LEN 3
#define ZEP_VERSION 2


/*
 * RFC 5905                   NTPv4 Specification                 June 2010
 */
/* 1970 - 1900 in seconds */
#define JAN_1970        2208988800UL
/* 2^32 as an int */
#define FRAC       (((uint64_t)1) << 32)

static const uint8_t magic[] = { 'E', 'X', ZEP_VERSION };
static void sniff_rx(phy_status_t status);
static uint8_t current_channel = 0;

static void timestamp_handler(uint32_t* seconds, uint32_t* subseconds)
{
    gps_synced_time_t time_now;
    gps_synced_clock_get(&time_now);

    *seconds = time_now.s;
    *subseconds = time_now.ms*1000 + time_now.us;
    return;
}

void init_sniffer(uint8_t channel)
{
    log_info("ZEPv2 Sniffer with channel: %u", channel);
    phy_set_channel(platform_phy, channel);
    register_timestamp_handler(timestamp_handler);
    current_channel = channel;
    sniff_rx(~0);  // call without a valid status to start rx
}

static void sniff_rx(phy_status_t status)
{

    static uint16_t device_id = 0; // TODO handle later

    /*
     * Two packets handling
     */
    static struct {
        int index;
        phy_packet_t pkt_buf[2];
    } sniff = {
        .index = 0,
        .pkt_buf = {
            // static version of phy_preprare_packet
            [0] = { .data = sniff.pkt_buf[0].raw_data },
            [1] = { .data = sniff.pkt_buf[1].raw_data },
        },
    };
    // get current packet and toggle next packet
    phy_packet_t *rx_pkt = &sniff.pkt_buf[sniff.index];
    sniff.index = (sniff.index + 1) % 2;
    // Enter RX again
    phy_status_t ret = phy_rx_now(platform_phy, &sniff.pkt_buf[sniff.index], sniff_rx);
    if (PHY_SUCCESS != ret)
        log_error("PHY RX FAILED");

    log_debug("status: %u", status);
    log_debug("sniff.index: %u", sniff.index);


    /*
     * Handle received packet and dump it to serial line
     */
    if (PHY_SUCCESS != status)
        return;

    // Data[0] == payload size, data[1:] payload
    static uint8_t data[256];
    uint8_t zep_pkt_len = to_zep(&data[1], rx_pkt, current_channel, device_id);
    data[0] = zep_pkt_len;

    uart_transfer(uart_print, data, 1 + zep_pkt_len);
    log_debug("uart transferted");
}
