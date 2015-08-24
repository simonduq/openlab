#include "platform.h"
#include "printf.h"
#include "soft_timer.h"


/*
 * A firmware that consums power and print a message every five minutes
 * The goal is to know when the battery will be discharged
 */

#define _DELAY_M   5
#define DELAY      soft_timer_s_to_ticks(60 * _DELAY_M)
#define PERIODIC   1

void do_serial_out()
{
    struct soft_timer_timeval time = soft_timer_time_extended();
	printf("Uptime %u\n", time.tv_sec);
}

int main()
{
    static soft_timer_t timer;
	platform_init();
	soft_timer_init();

    leds_on(0xFF);

    do_serial_out();
	soft_timer_set_handler(&timer, (handler_t)do_serial_out, NULL);
	soft_timer_start(&timer, DELAY, PERIODIC);
	platform_run();

	return 0;
}
