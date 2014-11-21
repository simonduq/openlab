#ifndef RADIO_NETWORK_H
#define RADIO_NETWORK_H
#include "config.h"
#include "computing.h"

enum packet_type {
    PKT_ACK = 0,
    PKT_GRAPH = 1,
    PKT_VALUES = 2,
};

extern uint32_t num_neighbours;
extern uint16_t neighbours[MAX_NUM_NEIGHBOURS];


// Init and resets the network layer, forgets current neighbours
void network_init(uint32_t channel, uint32_t discovery_tx_power,
        uint32_t communication_tx_power);

// Resets current neighbours
void network_reset();


// Broadcast messages at low power to make you known to your neighbours
void network_neighbours_discover(void);
// Send acknowledge messages to your neighbours nodes
// to improve the neighbours connection symmetry
void network_neighbours_acknowledge(void);

// Print the network neighbours table
void network_neighbours_print();

// Send my values to the neighbours
void network_send_values(uint8_t should_compute, struct values *values);

#endif//RADIO_NETWORK_H
