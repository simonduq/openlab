#include <stdio.h>
#include <math.h>
#include "detectpeak.h"


/* {window_size, peak_tempo, threshold */
count_peak_config_t PEAK_PARAM ={10, 50, 1.0} ;

void peak_setparam(count_peak_config_t peak_params)
{
  /* Params verification */
  if (peak_params.window_size < 0)
    peak_params.window_size = 0;
  else if (peak_params.window_size >  WINDOWS_MAX )
    peak_params.window_size =  WINDOWS_MAX;
  if (peak_params.peak_tempo < 0)
    peak_params.peak_tempo = 0;
  /* Set params */
  PEAK_PARAM.window_size = peak_params.window_size;
  PEAK_PARAM.peak_tempo  = peak_params.peak_tempo;
  PEAK_PARAM.threshold   = peak_params.threshold;
}

void detect_peak(int k, float sig[3], float *rpeak)
{
  static float sum, norm, norm_trace[WINDOWS_MAX], moy, dmoy, moy_trace, peak;
  static short int sign, sign_trace, k_trace, step; 

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
    k_trace = 0;
  } 
  else if (k < PEAK_PARAM.window_size) {
    sum = sum + norm;
    moy = sum / (k+1);
  }
  else {
    sum = sum + norm - norm_trace[k%PEAK_PARAM.window_size];
    moy = sum / PEAK_PARAM.window_size;
  }
  /*
   2 - Search Peak 
  */
  /* Compute derivative signal */
  if (k==0) {
    k_trace = 0;
    dmoy = 0.0;
    if (dmoy >=0 )
      sign_trace = 1;
    peak = 0;
  }
  else { 
    /* Compute derivative signal */
    dmoy = moy - moy_trace;
  }
  /* Compute the sign of the derivative signal */
  if (dmoy >=0 ) 
    sign = 1;
  else
    sign = -1; 
  /* Compute the switch of the sign with a signal threshold */
  if ( (sign_trace == 1) & (sign == -1) & (moy > PEAK_PARAM.threshold) 
       & ( k > (k_trace + PEAK_PARAM.peak_tempo) ) ) {
    peak = moy; 
    step ++;
    k_trace = k;
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
  norm_trace[k%PEAK_PARAM.window_size] = norm; 
  moy_trace = moy;
  sign_trace = sign;
}


void detect_peak_2(int k, float sig[3], float *rpeak)
{
  static float sum, norm, norm_trace[WINDOWS_MAX], moy, dmoy, moy_trace, peak;
  static short int sign, sign_trace, k_trace, step; 

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
    k_trace = 0;
  } 
  else if (k < PEAK_PARAM.window_size) {
    sum = sum + norm;
    moy = sum / (k+1);
  }
  else {
    sum = sum + norm - norm_trace[k%PEAK_PARAM.window_size];
    moy = sum / PEAK_PARAM.window_size;
  }
  /*
   2 - Search Peak 
  */
  /* Compute derivative signal */
  if (k==0) {
    k_trace = 0;
    dmoy = 0.0;
    if (dmoy >=0 )
      sign_trace = 1;
    peak = 0;
  }
  else { 
    /* Compute derivative signal */
    dmoy = moy - moy_trace;
  }
  /* Compute the sign of the derivative signal */
  if (dmoy >=0 ) 
    sign = 1;
  else
    sign = -1; 
  /* Compute the switch of the sign with a signal threshold */
  if ( (sign_trace == 1) & (sign == -1) & (moy > PEAK_PARAM.threshold) 
       & ( k > (k_trace + PEAK_PARAM.peak_tempo) ) ) {
    peak = moy; 
    step ++;
    k_trace = k;
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
  norm_trace[k%PEAK_PARAM.window_size] = norm; 
  moy_trace = moy;
  sign_trace = sign;
}
