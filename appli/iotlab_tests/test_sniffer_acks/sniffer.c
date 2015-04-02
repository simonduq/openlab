#include <string.h>
#include "platform.h"

#include "event.h"


#define LOG_LEVEL LOG_LEVEL_INFO
#include "debug.h"

void print_time(char *prefix, int32_t ticks)
{
    int32_t t_s = ticks / 32768;
    int32_t t_us = (int32_t)((((int64_t)(ticks % 32768)) * 1000000) / 32768);
    printf("%s %6d.%06d\n", prefix, t_s, t_us);
}

static xQueueHandle sniff_pkts;
static void sniff_handler(handler_arg_t arg)
{
    static phy_packet_t *prev_pkt = NULL;
    phy_packet_t *pkt = arg;
    printf("Packet len: %u\n", pkt->length);

    print_time("start    ", pkt->timestamp);
    print_time("stop     ", pkt->eop_time);
    print_time("rx_start ", pkt->t_rx_start);
    print_time("rx_end   ", pkt->t_rx_end);

    print_time("diff     ", pkt->eop_time - pkt->timestamp);
    print_time("diff_read", pkt->t_rx_end - pkt->t_rx_start);

    print_time("diff_st  ", pkt->t_rx_start - pkt->eop_time);
    print_time("diff_rx  ", pkt->t_rx_end - pkt->t_rx_start);

    if (pkt->length == 3) {
        print_time("diff_last            ", pkt->timestamp - prev_pkt->eop_time);

        print_time("diff_start | rx_start", pkt->timestamp - prev_pkt->t_rx_start);
        print_time("diff_start | rx_end  ", pkt->timestamp - prev_pkt->t_rx_end);


        printf("\n\n");
    }




    if (prev_pkt)
        xQueueSend(sniff_pkts, &prev_pkt, 0);
    prev_pkt = pkt;
    printf("\n");
}


void init_sniffer(uint8_t channel)
{
    log_info("Sniffer with channel: %u", channel);
    int i;
    sniff_pkts = xQueueCreate(16, sizeof(phy_packet_t *));
    static phy_packet_t pkts[16];
    for (i=0; i < 16; i++) {
        phy_packet_t *pkt = &pkts[i];
        phy_prepare_packet(pkt);
        xQueueSend(sniff_pkts, &pkt, 0);
    }

    log_info("Sniffer with channel: %u", channel);
    phy_set_channel(platform_phy, channel);
    phy_status_t status = phy_sniff(platform_phy, sniff_pkts, sniff_handler);
    log_info("phy_sniff: %d", status);
}
