#ifndef PHY_POWER_C_H
#define PHY_POWER_C_H

#include "string.h"
#include "phy.h"

struct _power_assoc {
    const char *name;
    const phy_power_t power;
};

static const char *radio_power_rf231_str = "-17dBm -12dBm -9dBm -7dBm"\
                                            " -5dBm -4dBm -3dBm -2dBm"\
                                            " -1dBm 0dBm 0.7dBm 1.3dBm"\
                                            " 1.8dBm 2.3dBm 2.8dBm 3dBm";

static const struct _power_assoc _power_dict[] = {
    {"-17dBm", PHY_POWER_m17dBm},
    {"-12dBm", PHY_POWER_m12dBm},
    {"-9dBm", PHY_POWER_m9dBm},
    {"-7dBm", PHY_POWER_m7dBm},
    {"-5dBm", PHY_POWER_m5dBm},
    {"-4dBm", PHY_POWER_m4dBm},
    {"-3dBm", PHY_POWER_m3dBm},
    {"-2dBm", PHY_POWER_m2dBm},
    {"-1dBm", PHY_POWER_m1dBm},
    {"0dBm", PHY_POWER_0dBm},
    {"0.7dBm", PHY_POWER_0_7dBm},
    {"1.3dBm", PHY_POWER_1_3dBm},
    {"1.8dBm", PHY_POWER_1_8dBm},
    {"2.3dBm", PHY_POWER_2_3dBm},
    {"2.8dBm", PHY_POWER_2_8dBm},
    {"3dBm", PHY_POWER_3dBm},
    {NULL, 0},
};

static unsigned int parse_power_rf231(const char *power_str)
{
    /* valid PHY_POWER_ values for rf231 */
    const struct _power_assoc *cur;
    for (cur = _power_dict; cur->name; cur++)
        if (strcmp(cur->name, power_str) == 0)
            return cur->power;
    return 255;
}

#endif//PHY_POWER_C_H
