#ifndef COMPUTING_H
#define COMPUTING_H
#include <stdint.h>
#include <math.h>


#define MAX_NUM_VALUES 32
#define MAX_NUM_NEIGHBOURS 64
#define MIN_RSSI -92

extern uint8_t num_values;
extern uint8_t compute_number;

struct received_values {
    uint32_t valid;
    uint32_t num_neighbours;
    double values[MAX_NUM_VALUES];
};


extern double my_values[MAX_NUM_VALUES];

extern uint32_t num_neighbours;
extern uint16_t neighbours[MAX_NUM_NEIGHBOURS];
extern struct received_values neighbours_values[MAX_NUM_NEIGHBOURS];


double init_value();
double compute_neighbours_value(double my_value, uint32_t n_neighbours,
        struct received_values *neighbors_values, uint8_t value_num);

double compute_received_value(double my_value, double neigh_value);
uint32_t compute_final_value();


#endif//COMPUTING_H
