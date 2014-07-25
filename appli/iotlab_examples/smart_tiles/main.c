/*
 * Smart tiles
 *
 */

#include <platform.h>
#include <printf.h>
#include <string.h>
#include <math.h>

#include "lsm303dlhc.h"
#include "l3g4200d.h"
#include "event.h"
#include "detectpeak.h"

// timer alarm function
static void alarm(handler_arg_t arg);
static soft_timer_t tx_timer;
/* Period of the sensor acquisition datas */
#define ACQ_PERIOD soft_timer_ms_to_ticks(5)
/* times of computation before transmit a result */
/* period in sec = (ACQ_PERIOD=5 x TX_COMPUTE) / 1000 */
#define TX_PERIOD 200

//event handler
static void handle_ev(handler_arg_t arg);

#define ACC_RES (1e-3)  // The resolution is 1 mg for the +/-2g scale
#define GYR_RES (8.75e-3)  // The resolution is 8.75mdps for the +/-250dps scale
#define GRAVITY 9.81
#define CALIB_PERIOD 100 // period in sec = CALIB_PERIOD=5 x TX_COMPUTE) / 1000 

/** Global counters structure */
typedef struct TypCounters {
 /* index incremented at each criteria computation */
  uint32_t index;
  /* local index incremented at each computation between 2 packet sending*/
  uint16_t lindex;
} TypCounters;

TypCounters glob_counters = {0, 0};

static void hardware_init()
{
    // Openlab platform init
    platform_init();
    event_init();
    soft_timer_init(); 
    // Switch off the LEDs
    leds_off(LED_0 | LED_1 | LED_2);
    // LSM303DLHC accelero sensor initialisation
    lsm303dlhc_powerdown();
    lsm303dlhc_acc_config(
			  LSM303DLHC_ACC_RATE_400HZ,	
			  LSM303DLHC_ACC_SCALE_2G,	
			  LSM303DLHC_ACC_UPDATE_ON_READ);
    // L3G4200D gyro sensor initialisation
    // l3g4200d_powerdown();
    // l3g4200d_gyr_config(L3G4200D_200HZ, L3G4200D_250DPS, true);
    // Initialize a openlab timer
    soft_timer_set_handler(&tx_timer, alarm, NULL);
    soft_timer_start(&tx_timer, ACQ_PERIOD, 1);

}

int main()
{
	hardware_init();
	platform_run();
	return 0;
}

static void handle_ev(handler_arg_t arg)
{
  int16_t a[3];
  //  int16_t g[3];
  int16_t i;
  float af[3];
  float peak;
  float norm;
  static float scale, normk;

  /* Read accelerometers */ 
  lsm303dlhc_read_acc(a);
  /* Read gyrometers */
  //l3g4200d_read_rot_speed(g);

  /* Sensors calibration during CALIB_PERIOD*/
  if (glob_counters.index <= CALIB_PERIOD) {
    /* Scale Accelero */
    scale = GRAVITY;
    if (glob_counters.index == 0)
      norm = 0.0;
    normk = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
    normk = sqrt(normk);
    norm += normk;
    if (glob_counters.index == CALIB_PERIOD) {
      norm = norm / CALIB_PERIOD;
      scale = GRAVITY / norm;
    }
  }

  
  /* Conversion */
  for (i=0; i < 3; i++) {
    af[i] = a[i] * ACC_RES * scale;
  }
  /* Peak detection */
  detect_peak(glob_counters.index, af, &peak);
  glob_counters.index++;

  if (peak > 0.0) {
    printf("Peak;0.0;0.0;%f\n", peak);
  }

  if (glob_counters.lindex == TX_PERIOD) {
    printf("Acc;%f;%f;%f\n", af[0], af[1], af[2]);
    // printf("Gyr;%f;%f;%f\n", g[0] * GYR_RES, g[1] * GYR_RES, g[2] * GYR_RES);
    glob_counters.lindex=0;
  }
  else {
    glob_counters.lindex++;
  }
}

static void alarm(handler_arg_t arg) {
  event_post_from_isr(EVENT_QUEUE_APPLI, handle_ev, NULL);
}
