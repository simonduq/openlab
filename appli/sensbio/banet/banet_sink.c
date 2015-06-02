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
 * banet_sink.c
 *
 * \brief banet TDMA coordinator
 *
 * \date Jan 06, 2015
 * \author: <roger.pissard.at.inria.fr>
 */

#include "platform.h"
#include "packet.h"
#include "soft_timer.h"

#include "mac_tdma.h"
#include "sensor.h"

#include "debug.h"

#pragma GCC diagnostic ignored "-Wcast-align"

/*
 * coordinator configuration:
 * + network ID 0x6666
 * + channel 21
 * + 4 slots (=> 2 client nodes) of 10ms each
 */
static mac_tdma_coord_config_t cfg = {
    /* network id */
    .panid = 0x6666,
    /* phy channel */
    .channel = 21,
    /* slot duration in tdma time unit (default 1unit = 100us) */
    .slot_duration = 100,
    /* number of slots (the coordinator can handle up to count-2 nodes) */
    .slot_count = 4,
};

static soft_timer_t timer;
uint32_t internal_time = 0;

static void pkt_tick(handler_arg_t arg);
static void pkt_received(packet_t *packet, uint16_t src);

static imu_sensor_data_t * rx_data;

int main()
{
    platform_init();

    /* Time initialization */
    internal_time = 0;

    /* init tdma */
    mac_tdma_init();

    /* start coordinator */
    mac_tdma_start_coord(&cfg);

    /* register data packet handler */
    mac_tdma_set_recv_handler(pkt_received);
   
    /*
     * programm periodic timer to send packet
     */
    soft_timer_set_handler(&timer, pkt_tick, NULL);
    soft_timer_start(&timer, soft_timer_ms_to_ticks(10), 1);

    /* shutdown leds */
    leds_off(0xf);

    platform_run();
    return 0;
}
 

static void pkt_received(packet_t *packet, uint16_t src)
{
    // unused
    (void) src;

    if (packet->length ==  sizeof(*rx_data)) {
      rx_data = (imu_sensor_data_t *) packet->data;
      log_printf("%04x:%u\t%u\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n", \
		 src, internal_time, rx_data->seq,		       \
		 rx_data->acc[0], rx_data->acc[1], rx_data->acc[2], \
		 rx_data->mag[0], rx_data->mag[1], rx_data->mag[2], \
		 rx_data->gyr[0], rx_data->gyr[1], rx_data->gyr[2]);
    }
    else {
      log_printf("Unknown Packet received from 0x%04x\n", src);
    }
    /*
    if (packet->length == 1)
    {
        log_printf("Packet received from 0x%04x : %u\n", src, *(packet->data));
    }
    else
    {
        log_printf("Unknown Packet received from 0x%04x\n", src);
    }
    */
    packet_free(packet);
}


static void pkt_tick(handler_arg_t arg)
{
  internal_time ++;
}
