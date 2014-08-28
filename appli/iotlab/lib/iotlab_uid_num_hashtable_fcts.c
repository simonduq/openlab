#include "iotlab_uid_num_hashtable.h"
#include <stdlib.h>


struct node node_from_uid(uint16_t uid)
{
    struct node found_node = {0, 0};
    const struct node_id_hash const *current = NULL;

    uint32_t hash = 0;
    hash ^= 0xf & (uid >> 0);
    hash ^= 0xf & (uid >> 4);
    hash ^= 0xf & (uid >> 8);
    hash ^= 0xf & (uid >> 12);

    for (current = nodes_uid_hash_table[hash]; current->hash != 0; current++) {
        // list is sorted in increasing order
        if (current->hash < uid)
            continue;
        if (current->hash == uid) {
            found_node.node_type = 0xf & (current->node >> 24);
            found_node.node_num  = 0x0fff & current->node;
        }
        break;  // current->hash >= uid so we can quit
    }
    return found_node;
}
