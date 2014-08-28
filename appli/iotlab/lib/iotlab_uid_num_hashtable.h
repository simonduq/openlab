/*
 * Generated from ./generate_uid_dict.py
 */

#include <stdint.h>

#define CC1101 0x1
#define CC2420 0x2
#define M3 0x3
#define A8 0x8

struct node_id_hash {
    uint16_t hash;
    uint32_t node;
};

struct node {
    uint8_t node_type;
    uint32_t node_num;
};

const struct node_id_hash * const nodes_uid_hash_table[16];


struct node node_from_uid(uint16_t uid);
