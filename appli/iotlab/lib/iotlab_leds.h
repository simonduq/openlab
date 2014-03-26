#ifndef IOTLAB_LEDS_H
#define IOTLAB_LEDS_H

#include <stdint.h>
#include "platform_leds.h"
#include "soft_timer.h"


/*
 * Toggle given leds 'leds_mask' with a period of 'period' ticks
 * It will toggle every period/2 ticks
 *
 * The current state of the leds is not reinitialized
 */
void leds_blink(soft_timer_t *timer, uint32_t period, uint8_t leds_mask);


/*
 * LEDS aliases
 */
#ifdef FITECO_M3
enum {
    LEDS_MASK  = LED_0 | LED_1 | LED_2,
    GREEN_LED  = LED_0,
    RED_LED    = LED_1,
    ORANGE_LED = LED_2,
};
#elif FITECO_A8
enum {
    LEDS_MASK  = LED_0 | LED_1 | LED_2,
    GREEN_LED  = LED_0,
    RED_LED    = LED_1,
    ORANGE_LED = LED_2,
};
#elif FITECO_GWT
enum {
    LEDS_MASK  = LED_0 | LED_1,
    GREEN_LED  = LED_0,
    RED_LED    = LED_1,
    ORANGE_LED = LED_2,
};
#endif


#endif //IOTLAB_LEDS_H
