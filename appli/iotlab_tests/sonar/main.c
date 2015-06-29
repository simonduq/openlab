/*
 *  Sonar
 *  Created on: Mar 4, 2015
 *      Author: Ana Garcia Alcala
 */

#include "platform.h"
#include <printf.h>

#include <string.h>

#include "mac_csma.h"
#include "phy.h"
#include "event.h"
#include "packer.h"
#include "unique_id.h"
#include "rf2xx.h"
#include "FreeRTOS.h"

// choose channel in [11-26]
#define CHANNEL 11
#define RADIO_POWER PHY_POWER_0dBm
#define PHY_MAX_TX_LENGTH 16
#define ADDR_BROADCAST 0xFFFF
extern rf2xx_t rf231;

static void print_usage();
static void reset_leds();
static void send_packet();
static void handle_cmd(handler_arg_t arg);
static void char_uart(handler_arg_t arg,uint8_t c);

static void send_packet() {
    uint16_t ret;
    static char packet[PHY_MAX_TX_LENGTH]; 
    uint16_t length;
    snprintf(packet, sizeof(packet), "Hello World!");
    length = 1 + strlen(packet);
    ret = mac_csma_data_send(ADDR_BROADCAST,(uint8_t *)packet,length);
     
     if (ret != 0){
        printf("packet sent\n");
        leds_on(LED_0); // Green led on
     }
     else{
        printf("packet sent failed\n");   
}}

/* Reception of a radio message */
void mac_csma_data_received(uint16_t src_addr,const uint8_t *c, uint8_t length, int8_t rssi, uint8_t lqi)
{
    uint16_t node_id;
    leds_on(LED_0);
    packer_uint16_unpack(c,&node_id);
    printf("%x;%04x;%d\n",src_addr,node_id,rssi);
    leds_on(LED_1); // Red led on
}	

static void reset_leds()
{
    leds_off(LED_0|LED_1|LED_2);
}

static void handle_cmd(handler_arg_t arg){
    uint8_t send = 0;
    phy_power_t power;

    switch((char) (uint32_t) arg) {
        case 'a':
            power = PHY_POWER_m17dBm;
            send = 1;
            break;
        case 'b':
            power = PHY_POWER_m12dBm;
            send = 1;
            break;
        case 'c':
            power = PHY_POWER_m7dBm;
            send = 1;
            break;
        case 'd':
            power = PHY_POWER_m3dBm;
            send = 1;
            break;
        case 'e':
            power = PHY_POWER_0dBm;
            send = 1;
            break;
        case 'f':                 
            power = PHY_POWER_3dBm;
            send = 1;
            break;
        case 'h':
            print_usage();
            break;
        case 'r':
            reset_leds();
            break;
            }
    
    if(send == 1){
        mac_csma_init(CHANNEL,power);
        send_packet();
    }    
}

static void char_uart(handler_arg_t arg,uint8_t c)
{
    event_post_from_isr(EVENT_QUEUE_APPLI,handle_cmd,(handler_arg_t)(uint32_t) c);
}

/*
 * HELP
 */
static void print_usage()
{
    printf("\n\nIoT-LAB Sonar test\n");
    printf("Type command\n");
    printf("\th:\tprint this help\n");
    printf("\ta:\tsend a broadcast message at -17 dBm \n");
    printf("\tb:\tsend a broadcast message at -12 dBm \n");
    printf("\tc:\tsend a broadcast message at -7  dBm \n");
    printf("\td:\tsend a broadcast message at -3  dBm \n");
    printf("\te:\tsend a broadcast message at  0  dBm \n");
    printf("\tf:\tsend a broadcast message at  3  dBm \n");
    printf("\th:\thelp print this help \n");
    printf("\tr:\treset leds \n");

}

int main (void) {
    	
	// Openlab platform init
	platform_init();
	event_init();
	
	// Switch off the LEDs
	leds_off(LED_0|LED_1|LED_2);
   	// Print usage help 
   	print_usage();
   	// Uart initialisation
    uart_set_rx_handler(uart_print,char_uart, NULL);

   	// Init csma Radio mac layer
    mac_csma_init(CHANNEL, RADIO_POWER);
    // Set RSSI threshold	
	phy_idle(platform_phy);
	rf2xx_set_rx_rssi_threshold(rf231,RF2XX_PHY_RX_THRESHOLD__m57dBm);

    platform_run();
    
    return 0;
}

