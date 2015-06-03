/*
 *  Sonar
 *  Created on: Mar 4, 2015
 *      Author: Ana Garcia Alcala
 */

#include <platform.h>
#include <printf.h>
#include <string.h>

#include "mac_csma.h"
#include "phy.h"
#include "event.h"
#include "packer.h"
#include "unique_id.h"
#include "FreeRTOS.h"

// choose channel in [11-26]
#define CHANNEL 11
#define RADIO_POWER PHY_POWER_0dBm
#define PHY_MAX_TX_LENGTH 16
 
#define ADDR_BROADCAST 0xFFFF

#define SM_IDLE  0
#define SM_RX    1
#define SM_TX    2
uint8_t sm = SM_IDLE;

static void send_packet();
uint16_t char_rx(uint8_t c);
static void handle_cmd(handler_arg_t arg);
static void char_uart(handler_arg_t arg,uint8_t c);
uint16_t node_id;

static void send_packet() {
        uint16_t ret;
        static uint8_t num = 0;
        static char packet[PHY_MAX_TX_LENGTH ]; 
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
}	

uint16_t char_rx(uint8_t c) {
 
            char data [2]; 
            data[0] = c;
            printf("INPUT -> %s\n",data );

            switch(c) {
                case 'a':
		 	mac_csma_init(CHANNEL,PHY_POWER_m17dBm);  
		 	send_packet();
                        break;
                case 'b':
			mac_csma_init(CHANNEL,PHY_POWER_m12dBm);
			send_packet();       		
  			break;
                case 'c':
		    	mac_csma_init(CHANNEL,PHY_POWER_m7dBm);
		  	send_packet();
  		   	break;
                case 'd':
	            	mac_csma_init(CHANNEL,PHY_POWER_m3dBm);
		   	send_packet();
                    	break;
                case 'e':
		    	mac_csma_init(CHANNEL,PHY_POWER_0dBm);
		     	send_packet();
                    	break;
                case 'f':                 
	            	mac_csma_init(CHANNEL,PHY_POWER_3dBm);
		     	send_packet();
			break;
                case 'g':
		     	mac_csma_init(CHANNEL,PHY_POWER_0dBm);
 		     	send_packet();
	             	break;
                case 'h':
		   	mac_csma_init(CHANNEL,PHY_POWER_0dBm);
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


int main (void) {
       // Openlab platform init
	platform_init();
	event_init();

	// Switch off the LEDs
	leds_off(LED_0 | LED_1 | LED_2);
	// Uart initialisation
	uart_set_rx_handler(uart_print,char_uart, NULL);

	// Init csma Radio mac layer
	mac_csma_init(CHANNEL, RADIO_POWER);
        platform_run();
    return 0;
}

