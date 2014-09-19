#include <stdio.h>
#include <math.h>
#include "detectpeak.h"


void peak_setparam(count_peak_config_t trace, short int window_size, short int peak_tempo, float threshold)
{
  /* Params verification before set */
  if (window_size < 0)
    trace.window_size  = 0;
  else if (window_size >  WINDOWS_MAX)
    trace.window_size =  WINDOWS_MAX;
  else
    trace.window_size = window_size;

  if (peak_tempo < 0)
    peak_tempo = 0;
  else
    trace.peak_tempo  = peak_tempo;

  trace.threshold   = threshold;
}


void peak_detect(count_peak_config_t trace, int k, float sig[3], float *rpeak)
{
  static float sum, norm,  moy, dmoy, peak;
  static short int sign, step; 

  /*
    0 - Choose the signal to detect
   */
  /*  norm = sig[2]; */
  norm = sig[0]*sig[0] + sig[1]*sig[1] + sig[2]*sig[2];
  norm = sqrt(norm);
  /* 
   1 - Moving Average 
  */
  if (k==0) {
    sum = norm;
    moy = norm;
    step = 0;
    trace.k = 0;
  } 
  else if (k < trace.window_size) {
    sum = sum + norm;
    moy = sum / (k+1);
  }
  else {
    sum = sum + norm - trace.norm[k%trace.window_size];
    moy = sum / trace.window_size;
  }
  /*
   2 - Search Peak 
  */
  /* Compute derivative signal */
  if (k==0) {
    trace.k = 0;
    dmoy = 0.0;
    if (dmoy >=0 )
      trace.sign = 1;
    peak = 0;
  }
  else { 
    /* Compute derivative signal */
    dmoy = moy - trace.moy;
  }
  /* Compute the sign of the derivative signal */
  if (dmoy >=0 ) 
    sign = 1;
  else
    sign = -1; 
  /* Compute the switch of the sign with a signal threshold */
  if ( (trace.sign == 1) & (sign == -1) & (moy > trace.threshold) 
       & ( k > (trace.k + trace.peak_tempo) ) ) {
    peak = moy; 
    step ++;
    trace.k = k;
  }
  else
    peak = 0.0;

  /* return value */
  *rpeak = peak;

  /* DEBUG
  if ( peak > 0)
    printf("PEAK %d %d %f %f \n",k, step, moy, dmoy);
  */

 /* Store old values */
  trace.norm[k%trace.window_size] = norm; 
  trace.moy = moy;
  trace.sign = sign;
}


