#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <printf.h>
#include <string.h>

#include "phy.h"
#include "soft_timer.h"
#include "event.h"
#include "computing.h"
#include "iotlab_uid.h"

#include "mac_csma.h"
#include "phy.h"

// choose channel in [11-26]
#define CHANNEL 11
#define GRAPH_RADIO_POWER PHY_POWER_m17dBm
#define RADIO_POWER PHY_POWER_3dBm


#define ADDR_BROADCAST 0xFFFF

// UART callback function
static void char_rx(handler_arg_t arg, uint8_t c);
static void handle_cmd(handler_arg_t arg);

static uint8_t g_packet[PHY_MAX_TX_LENGTH - 4];  // 4 for mac layer


enum packet_type {
    PKT_ACK = 0,
    PKT_GRAPH = 1,
    PKT_VALUES = 2,
};

static void send(uint16_t addr, uint8_t *packet, size_t index)
{
    uint16_t ret;
    ret = mac_csma_data_send(addr, packet, index);
    if (ret != 0)
        printf("DEBUG:Packet sent to %04X\n", addr);
    else
        printf("DEBUG:Packet sent to %04X failed\n", addr);
}


static void send_values(uint8_t should_compute)
{
    uint16_t node_id = iotlab_uid();
    uint8_t *packet = g_packet;
    size_t index = 0;
    uint8_t pkt_type = PKT_VALUES;
    memset(packet, 0, sizeof(packet));

    // Packet content
    // pkt_type | should compute value | num_neighbours | values
    memcpy(&packet[index], &pkt_type, sizeof(pkt_type));
    index += sizeof(pkt_type);
    memcpy(&packet[index], &node_id, sizeof(node_id));
    index += sizeof(node_id);
    memcpy(&packet[index], &should_compute, sizeof(should_compute));
    index += sizeof(should_compute);
    memcpy(&packet[index], &num_neighbours, sizeof(num_neighbours));
    index += sizeof(num_neighbours);
    memcpy(&packet[index], my_values, num_values * sizeof(double));
    index += num_values * sizeof(double);

    send(ADDR_BROADCAST, packet, index);
}

static void send_graph_pkt(uint16_t addr);

static void create_graph()
{
    mac_csma_init(CHANNEL, GRAPH_RADIO_POWER);
    send_graph_pkt(ADDR_BROADCAST);
    mac_csma_init(CHANNEL, RADIO_POWER);
}

static void validate_graph()
{
    int i;
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        uint16_t neighbour_addr = neighbours[i];
        if (neighbour_addr == 0)
            break;
        send_graph_pkt(neighbour_addr);
        soft_timer_delay(soft_timer_ms_to_ticks(10));
    }
}

static void send_graph_pkt(uint16_t addr)
{
    uint8_t *packet = g_packet;
    size_t index = 0;

    uint8_t pkt_type = PKT_GRAPH;
    // packet content
    // pkt_type
    memset(packet, 0, sizeof(packet));
    memcpy(&packet[index], &pkt_type, sizeof(pkt_type));
    index += sizeof(pkt_type);

    send(addr, packet, index);
}

static void print_neighbours()
{
    int i;
    printf("%04x,Neighbours", iotlab_uid());
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        uint16_t neighbour_addr = neighbours[i];
        if (neighbour_addr == 0)
            break;
        printf(";%04x", neighbour_addr);
    }
    printf("\n");
}

static void print_values()
{
    int i;
    printf("%04x;Values;%u", iotlab_uid(), compute_number);
    for (i = 0; i < num_values; i++)
        printf(";%f", my_values[i]);
    printf("\n");
}

static void print_final_value()
{
    uint32_t final_value = compute_final_value();
    printf("%04x;FinalValue;%u;%u\n", iotlab_uid(),
            compute_number, final_value);
}


static void handle_graph(uint16_t src_addr, int8_t rssi)
{
    printf("GRAPH from %04x. rssi: %d\n", src_addr, rssi);

    if (rssi < MIN_RSSI)
        return;

    int i;
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        uint16_t neighbour_addr = neighbours[i];
        if (neighbour_addr == 0) {
            neighbours[i] = src_addr;
            num_neighbours++;
            break;
        } else if (neighbour_addr == src_addr) {
            break;
        } else {
            continue;
        }

    }
}


static int neighbour_id(uint16_t src_addr)
{
    int i;
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        uint16_t neighbour_addr = neighbours[i];
        if (neighbour_addr == src_addr)
            return i;
        else if (neighbour_addr == 0)
            return -1;
    }
    return -1;
}

static void compute_received_values(struct received_values *neigh_values)
{
    int i;
    compute_number++;
    for (i = 0; i < num_values; i++) {
        my_values[i] = compute_received_value(
                my_values[i], neigh_values->values[i]);
    }
}

static void handle_value(uint16_t src_addr, const uint8_t *data)
{
    int index = neighbour_id(src_addr);
    if (index == -1) {
        printf("VALUES from %04x: not neighbour\n", src_addr);
        return;
    } else {
        printf("VALUES from %04x\n", src_addr);
    }

    // pkt_type | should compute value | num_neighbours | values
    struct received_values *values;
    values = &neighbours_values[index];
    values->valid = 1;
    memcpy(&values->num_neighbours, &data[2], sizeof(uint32_t));
    memcpy(&values->values, &data[2 + sizeof(uint32_t)],
            num_values * sizeof(double));

    uint8_t compute = data[1];
    if (compute)
        compute_received_values(values);
}

static void compute_all_values()
{
    int i;
    compute_number++;

    num_neighbours = 0;
    for (i = 0; i < MAX_NUM_NEIGHBOURS; i++) {
        if (neighbours_values[i].valid)
            num_neighbours++;
    }
    for (i = 0; i < num_values; i++) {
        my_values[i] = compute_neighbours_value(
                my_values[i], num_neighbours, neighbours_values, i);
    }

    // Reset values for next run
    memset(neighbours_values, 0, sizeof(neighbours_values));
}



/* Reception of a radio message */
void mac_csma_data_received(uint16_t src_addr,
        const uint8_t *data, uint8_t length, int8_t rssi, uint8_t lqi)
{
    uint8_t pkt_type = data[0];

    switch (pkt_type) {
    case (PKT_ACK):
        printf("ACK from %04x\n", src_addr);
        break;
    case (PKT_GRAPH):
        handle_graph(src_addr, rssi);
        break;
    case (PKT_VALUES):
        handle_value(src_addr, data);
        break;
    default:
        printf("Unknown pkt type %01x from %04x\n", pkt_type, src_addr);
        break;
    }
}

void init_values()
{
    memset(my_values, 0, sizeof(my_values));
    int i;
    for (i = 0; i < num_values; i++)
        my_values[i] = init_value();
}

static void print_usage()
{
    printf("\n\nIoT-LAB Simple Demo program\n");
    printf("Type command\n");
    printf("\th:\tprint this help\n");
    printf("\n");
    printf("\tg:\tcreate connection graph for this node\n");
    printf("\tG:\tValidate Graph with neighbours\n");
    printf("\tp:\tprint neighbours table\n");
    printf("\n");
    printf("\ti:\tinit sensor value\n");
    printf("\n");
    printf("\ts:\tsend values to neighbours\n");
    printf("\tS:\tsend values to neighbours and make them compute a new value\n");
    printf("\n");
    printf("\tc:\tcompute values received from all neighbours\n");
    printf("\n");
    printf("\tv:\tprint current node values\n");
}

static void handle_cmd(handler_arg_t arg)
{
    switch ((char) (uint32_t) arg) {
        case 'c':
            compute_all_values();
            break;

        case 'g':
            create_graph();
            break;
        case 'G':
            validate_graph();
            break;
        case 'p':
            print_neighbours();
            break;

        case 'i':
            init_values();
            break;


        case 's':
            send_values(0);
            break;
        case 'S':
            send_values(1);
            break;
        case 'v':
            print_values();
            break;
        case 'V':
            print_final_value();
            break;
        case '\n':
            break;
        case 'h':
        default:
            print_usage();
            break;
    }
}

static void hardware_init()
{

    // Openlab platform init
    platform_init();
    event_init();
    soft_timer_init();

    // Switch off the LEDs
    leds_off(LED_0 | LED_1 | LED_2);

    // Uart initialisation
    uart_set_rx_handler(uart_print, char_rx, NULL);

    // Init csma Radio mac layer
    mac_csma_init(CHANNEL, RADIO_POWER);
}

int main()
{
    hardware_init();
    platform_run();
    return 0;
}


/* Reception of a char on UART and store it in 'cmd' */
static void char_rx(handler_arg_t arg, uint8_t c) {
    // disable help message after receiving char
    event_post_from_isr(EVENT_QUEUE_APPLI, handle_cmd,
            (handler_arg_t)(uint32_t) c);
}

