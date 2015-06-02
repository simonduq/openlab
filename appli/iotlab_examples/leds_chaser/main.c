#include "platform.h"

#define DELAY 0x160000

//Global variables
int j,i;

/*Time delay */
static void time_delay(unsigned int d)
{
 for (i = 0;i<DELAY;i++){
 
  __asm__("nop");
 }
}

/* Leds change */
static void led_change(int led_state)
{  
  leds_off(LED_0 | LED_1 | LED_2);
  
  switch(j)
  {
 
  case 0:
  case 5:
  leds_on(LED_2);  // Green led on
  break;

  case 1:
  case 4:
  leds_on(LED_1); // Red led on
  break;

  case 2:
  case 3:
  leds_on(LED_0); // Orange led on
  break;
  }
}

int main()

{
 /* Init the platform */
   platform_init();
   
   while(1)
   {

   for(j = 0;j<6;j++)
   {
   led_change(j);
   time_delay(DELAY);
   } }

  return 0;
}


