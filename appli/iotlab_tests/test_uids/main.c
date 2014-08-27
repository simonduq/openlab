#include "platform.h"
#include "soft_timer.h"
#include "unique_id.h"
#include "printf.h"
#include "iotlab_uid.h"

static void print_uids(void *args)
{
    printf("Chip UIDs extraction:\n");

    printf("FULL UID 32: %08x:%08x:%08x\n",
            uid->uid32[0],
            uid->uid32[1],
            uid->uid32[2]);
    printf("FULL UID 16: %04x%04x:%04x%04x:%04x%04x\n",
            uid->uid16[0], uid->uid16[1],
            uid->uid16[2], uid->uid16[3],
            uid->uid16[4], uid->uid16[5]);

    printf("FULL UID  8: %02x%02x%02x%02x:%02x%02x%02x%02x:%02x%02x%02x%02x\n",
            uid->uid8[0], uid->uid8[1],
            uid->uid8[2], uid->uid8[3],
            uid->uid8[4], uid->uid8[5],
            uid->uid8[6], uid->uid8[7],
            uid->uid8[8], uid->uid8[9],
            uid->uid8[10], uid->uid8[11]);

    printf("Processed UID used in platform:\n");
    printf("Platform 32b UUID: \n\t%02x:%02x:%02x:%02x\n",
            uid->uid8[8], uid->uid8[9],
            uid->uid8[10], uid->uid8[11]);

    // Extract a 16bit UUID uniq for all the platform
    // Keeping uid8[9], (uid8[8] | (uid8[10] << 7)) % 256
    //
    //     The validity has been tested on all nodes UID that passed autotest
    //
    printf("Extracted 16b UUID: \n\t%02x:%02x\n",
            (uid->uid8[8] | (uid->uid8[10] << 7)) % 256,
            uid->uid8[9]
            );

    uint16_t id = iotlab_uid();
    printf("iotlab_uid (16b):\n\t%02x:%02x\n", id >> 8, id & 0xFF);


    printf("platform_uid: 0x%x\n", platform_uid);
    if (platform_uid)
        printf("platform_uid() == 0x%04x\n", platform_uid());


    printf("\n");
}


int main(void)
{
    static soft_timer_t print_timer;

    platform_init();
    soft_timer_init();

    soft_timer_set_handler(&print_timer, print_uids, NULL);
    soft_timer_start(&print_timer, soft_timer_s_to_ticks(2), 1);

    platform_run();
    return 1;
}
