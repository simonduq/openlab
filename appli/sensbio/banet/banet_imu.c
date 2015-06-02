/*
* This file is a part of openlab/sensbiotk
*
* Copyright (C) 2015  INRIA (Contact: sensbiotk@inria.fr)
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * banet_node.c
 *
 * \brief banet TDMA node IMU
 *
 * \date Jan 06, 2015
 * \author: <roger.pissard.at.inria.fr>
 */

#include <stdint.h>
#include <string.h>
#include "platform.h"
#include "packet.h"
#include "soft_timer.h"

#include "mac_tdma.h"
#include "sensor.h"

#include "debug.h"

/*
 * node configuration:
 * + network ID 0x6666
 * + channel 21
 * + requested bandwidth (to the coordinator) 20 packets/second
 */
static mac_tdma_node_config_t cfg = {
    /* network id */
    .panid = 0x6666,
    /* bandwidth in pkt/s */
    .bandwidth = 20,
    /* phy channel */
    .channel = 21,
};

static uint16_t count;
static imu_sensor_data_t tx_data;

static void pkt_sent(void *arg, enum tdma_result res);
static void pkt_received(packet_t *packet, uint16_t src);

static void slot_callback(uint8_t id, uint32_t time);
static void slot_callback_send(handler_arg_t arg);


int main()
{
    platform_init();

    /* init IMU sensor */
    init_sensor();

    /* init tdma */
    mac_tdma_init();

    /* plug a call when you receive a TDMA slot */
    mac_tdma_set_slot_callback(slot_callback);

    /* start node */
    mac_tdma_start_node(&cfg);

    /* register data packet handler */
    mac_tdma_set_recv_handler(pkt_received);

    /* shutdown leds */
    leds_off(0xf);

    platform_run();
    return 0;
}

static void pkt_send()
{
    enum tdma_result res;

    /* ensure we're connected */
    if (mac_tdma_is_connected())
    {
        leds_on(LED_1);
    }
    else
    {
        leds_off(LED_1);
        return;
    }

    /* get a packet */
    packet_t *packet = packet_alloc(0);
    if (!packet)
    {
        log_error("Can't allocate a packet");
        return;
    }

    /* fill it */
    log_printf("Send packet %u\n", count);
    tx_data.seq = count++;
    read_sensor(&tx_data);

    packet->length = sizeof(tx_data);
    memcpy(packet->data, (void *) &tx_data,  packet->length);

    /* send the packet to the coordinator */
    if ((res = mac_tdma_send(packet, 0, pkt_sent, packet)) != TDMA_OK)
    {
        packet_free(packet);
        log_printf("Packet sending failed %d\n", res);
    }
}

static void pkt_sent(void *arg, enum tdma_result res)
{
    // unused
    (void) res;

    log_printf("Packet sending result %d\n", res);

    /* free packet */
    packet_free((packet_t *) arg);
}

static void pkt_received(packet_t *packet, uint16_t src)
{
    // unused
    (void) src;

    if (packet->length == 1)
    {
        log_printf("Packet received from 0x%04x : %u\n", src, *(packet->data));
        /* make the lend blink */
        if (*(packet->data) % 2)
        {
            leds_on(LED_0);
        }
        else
        {
            leds_off(LED_0);
        }
    }
    else
    {
        log_printf("Unknown Packet received from 0x%04x\n", src);
    }

    /* free the packet */
    packet_free(packet);
}

static void slot_callback(uint8_t id, uint32_t time)
{
    (void) time;
    event_post(EVENT_QUEUE_APPLI, slot_callback_send, (void *) (uintptr_t) id);
}

static void slot_callback_send(handler_arg_t arg)
{
    unsigned id = (uintptr_t) arg;
    (void) id;
    if (id == 0) {
      log_printf("Slot %u\n", id);
      pkt_send();
    }
}
