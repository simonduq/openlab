#include "printf.h"
#include "scanf.h"

#include "platform.h"
#include "soft_timer.h"

#include "unique_id.h"

#include "shell.h"


/* Leds Commands */
static int cmd_leds_on(int argc, char **argv)
{
    uint8_t leds = 0;
    if (argc != 2)
        return 1;

    if (1 != sscanf(argv[1], "%u", &leds))
        return 1;

    leds_on(leds);
    printf("%s %x\n", argv[0], leds);
    return 0;
}

static int cmd_leds_off(int argc, char **argv)
{
    uint8_t leds = 0;
    if (argc != 2)
        return 1;

    if (!sscanf(argv[1], "%u", &leds))
        return 1;

    leds_off(leds);
    printf("%s %x\n", argv[0], leds);
    return 0;
}


static int cmd_leds_blink(int argc, char **argv)
{
    uint8_t leds = 0;
    uint32_t time = 0;
    static soft_timer_t led_alarm;

    if (argc != 3)
        return 1;
    if (!sscanf(argv[1], "%u", &leds))
        return 1;
    if (!sscanf(argv[2], "%u", &time))
        return 1;

    if (time) {
        soft_timer_set_handler(&led_alarm, (handler_t)leds_toggle,
                (handler_arg_t)(uint32_t)leds);
        soft_timer_start(&led_alarm, soft_timer_ms_to_ticks(time), 1);
        printf("%s %u %u\n", argv[0], leds, time);
    } else {
        soft_timer_stop(&led_alarm);
        printf("%s stop\n", argv[0]);
    }
    return 0;
}


/* Simple Commands */

static int cmd_echo(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++)
        if (i == 1)
            printf("%s", argv[i]);
        else
            printf(" %s", argv[i]);
    printf("\n");
    return 0;
}

static int cmd_get_time(int argc, char **argv)
{
    if (argc != 1)
        return 1;
    printf("%s %u ticks_32khz\n", argv[0], soft_timer_time());
    return 0;
}

static int cmd_get_uid(int argc, char **argv)
{
    if (argc != 1)
        return 1;
    printf("%s %08x%08x%08x\n", argv[0],
            uid->uid32[0], uid->uid32[1], uid->uid32[2]);
    return 0;
}


struct shell_command commands[] = {
    {"echo",       "echo given arguments", cmd_echo},
    {"get_time",   "Print board time",     cmd_get_time},
    {"get_uid",    "Print board uid",      cmd_get_uid},

    {"leds_on",    "[leds_flag] Turn given leds on",  cmd_leds_on},
    {"leds_off",   "[leds_flag] Turn given leds off", cmd_leds_off},
    {"leds_blink", "[leds_flag] [time] Blink leds every 'time'. If 'time' == 0 disable", cmd_leds_blink},
    {NULL, NULL, NULL},
};


int main(void)
{
    platform_init();
    soft_timer_init();
    event_init();
    shell_init(commands, 1);
    platform_run();
    return 1;
}

