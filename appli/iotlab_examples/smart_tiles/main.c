/*
 * Smart tiles
 *
 */

#include <platform.h>
#include <printf.h>
#include <string.h>
#include <math.h>

#include "lsm303dlhc.h"
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

count_peak_config_t PEAKACC_TRACE;
count_peak_config_t PEAKMAG_TRACE;

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
    // LSM303DLHC magneto sensor initialisation
    lsm303dlhc_mag_config(LSM303DLHC_MAG_RATE_220HZ,
                          LSM303DLHC_MAG_SCALE_2_5GAUSS, 
			  LSM303DLHC_MAG_MODE_CONTINUOUS,
                          LSM303DLHC_TEMP_MODE_ON);
    // Set peak detection parameters: windows size, peak_tempo, threshold */
    peak_setparam(PEAKACC_TRACE,10, 50, 1.0);
    peak_setparam(PEAKMAG_TRACE,10, 50, 1.0);
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
  int16_t m[3];
  int16_t i;
  float af[3], mf[3];
  float accpeak;
  float magpeak;
  float accnorm, magnorm;
  static float accscale, accnormk;
  static float magscale, magnormk;

  /* Read accelerometers */ 
  lsm303dlhc_read_acc(a);
  /* Read magnetometers */ 
  lsm303dlhc_read_mag(m);
  /* Sensors calibration during CALIB_PERIOD*/
  if (glob_counters.index <= CALIB_PERIOD) {
    /* Scale Accelero and magneto*/
    accscale = GRAVITY;
    magscale = 1.0;
    /* first index */
    if (glob_counters.index == 0)
      {
	accnorm = 0.0;
	magnorm = 0.0;
      }
    /* computation */
    accnormk = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
    accnormk = sqrt(accnormk);
    accnorm += accnormk;
    magnormk = m[0] * m[0] + m[1] * m[1] + m[2] * m[2];
    magnormk = sqrt(magnormk);
    magnorm += magnormk;
    /* last index */
    if (glob_counters.index == CALIB_PERIOD) {
      accnorm = accnorm / CALIB_PERIOD;
      accscale = GRAVITY / accnorm;
      magnorm = magnorm / CALIB_PERIOD;
      magscale = 1.0 / magnorm;
    }
    glob_counters.index++;
  } /* After calibration */
  else {
    /* Conversion */
    for (i=0; i < 3; i++) {
      af[i] = a[i] * ACC_RES * accscale; 
      mf[i] = m[i] * magscale;
    } 
    /* Peaks detection after calibration*/ 
    peak_detect(PEAKACC_TRACE, glob_counters.index, af, &accpeak); 
    peak_detect(PEAKMAG_TRACE, glob_counters.index, mf, &magpeak);

    glob_counters.index++;
    /* Printing */
    if (accpeak > 0.0) {
      printf("AccPeak;0.0;0.0;%f\n", accpeak);
    }
    if (magpeak > 0.0) {
      printf("MagPeak;0.0;0.0;%f\n", magpeak);
    }
    if (glob_counters.lindex == TX_PERIOD) {
      printf("Acc;%f;%f;%f\n", af[0], af[1], af[2]);
      printf("Mag;%f;%f;%f\n", mf[0], mf[1], mf[2]);
      printf("Mag1;%d;%d;%d;%f\n", m[0], m[1], m[2], magscale);
      glob_counters.lindex=0;
    }
    else {
      glob_counters.lindex++;
    }
  }
}

static void alarm(handler_arg_t arg) {
  event_post_from_isr(EVENT_QUEUE_APPLI, handle_ev, NULL);
}
