
/* Gn.c
 *
 *  Created on: Mar 4, 2015
 World
 *      Author: iotlab
 */

#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include <string.h>

#include "mac_csma.h"
#include "phy.h"
#include  "timer.h"
#include "event.h"
#include "packer.h"
#include "unique_id.h"
#include "FreeRTOS.h"
#include "semphr.h"

#ifdef IOTLAB_M3
#include "lps331ap.h"
#include "isl29020.h"
#endif

// choose channel in [11-26]
#define CHANNEL 11
#define RADIO_POWER PHY_POWER_0dBm
#define PHY_MAX_TX_LENGTH 16 
 
#define ADDR_BROADCAST 0xFFFF

#define SM_IDLE  0
#define SM_RX    1
#define SM_TX    2
volatile uint8_t sm = SM_IDLE;

#if 0
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

uint16_t packet_error(void);
uint16_t packet_sent(void);
static void send_packet();
uint16_t char_rx(uint8_t c);
static void handle_cmd(handler_arg_t arg);
static void char_uart(handler_arg_t arg,uint8_t c);
uint16_t node_id;

// Global variables
volatile int16_t data_rx, data_tx; // flags to know when events happened
uint8_t txframe[32]; // frame to send
uint8_t rxframe[32]; // received frame
uint16_t rxfrom, rxlen; // information on received frame
int16_t rxrssi; // received frame rssi

uint8_t tx_c;
uint8_t send = 0;
uint16_t txlength;
uint8_t table[1];

static void send_packet() {
        uint16_t ret;
        static uint8_t num = 0;
        static char packet[PHY_MAX_TX_LENGTH - 4]; // 4 for mac layer
        uint16_t length;
        snprintf(packet, sizeof(packet), "Hello World!: %u", num++);
        length = 1 + strlen(packet);
        ret = mac_csma_data_send(ADDR_BROADCAST,(uint8_t *)packet,length);

      
         if (ret != 0)
          printf("packet sent\n");
         else
          printf("Packet sent failed\n");   
    }

/* Reception of a radio message */
void mac_csma_data_received(uint16_t src_addr,const uint8_t *c, uint8_t length, int8_t rssi, uint8_t lqi)
{

       leds_on(LED_0);

        packer_uint16_unpack(c,&node_id);
	printf("%x;%04x;%d\n",src_addr,node_id,rssi);
	handle_cmd((handler_arg_t) '\n');
}	


uint16_t packet_sent(void) {
        return 0;
    }

uint16_t packet_error(void) {
        return 0;

}

uint16_t char_rx(uint8_t c) {

            switch(c) {
                case 'a':
		 	mac_csma_init(CHANNEL,PHY_POWER_m30dBm);  
		 	send_packet();
                 	break;
                case 'b':
			mac_csma_init(CHANNEL,PHY_POWER_m25dBm);
			send_packet();       		
  			break;
                case 'c':
		    	mac_csma_init(CHANNEL,PHY_POWER_m20dBm);
		  	send_packet();
  		   	break;
                case 'd':
	            	mac_csma_init(CHANNEL,PHY_POWER_m15dBm);
		   	send_packet();
                    	break;
                case 'e':
		    	mac_csma_init(CHANNEL,PHY_POWER_m10dBm);
		     	send_packet();
                    	break;
                case 'f':                 
	            	mac_csma_init(CHANNEL,PHY_POWER_m5dBm);
		     	send_packet();
			break;
                case 'g':
		     	mac_csma_init(CHANNEL,PHY_POWER_0dBm);
 		     	send_packet();
	             	break;
                case 'h':
		   	mac_csma_init(CHANNEL,PHY_POWER_5dBm);
		   	send_packet();
                     	break;
            }
            
            if(c != 'r') {
				sm = SM_TX;
				return 1;
			}
			else
			{
				leds_off(LED_0|LED_1|LED_2);
				return 0;
			}
 
}

static void handle_cmd(handler_arg_t arg){
            char_rx((char) (uint32_t) arg);
}

static void char_uart(handler_arg_t arg,uint8_t c)
{
 event_post_from_isr(EVENT_QUEUE_APPLI,handle_cmd,(handler_arg_t)(uint32_t) c);
}

static void hardware_init()
{
	// Openlab platform init
	platform_init();
	event_init();

	// Switch off the LEDs
	leds_off(LED_0 | LED_1 | LED_2);
	// Uart initialisation
	uart_set_rx_handler(uart_print,char_uart, NULL);

	// Init csma Radio mac layer
	mac_csma_init(CHANNEL, RADIO_POWER);
}



int main (void) {


	hardware_init();
        platform_run();

    while(1)
    {
        phy_idle(platform_phy);
        
	if(sm == SM_TX)
        {
            static uint8_t msg[64];
            int len;
      	    leds_on(LED_1);
            len = printf((char*)msg,"%.4x",node_id);
            mac_csma_data_send(ADDR_BROADCAST,msg,len);
            sm = SM_IDLE;
        }
    }

return 0;
}

