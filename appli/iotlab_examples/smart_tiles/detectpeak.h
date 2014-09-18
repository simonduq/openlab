#ifndef _DETECTPEAK_H
#define _DETECTPEAK_H

#define WINDOWS_MAX 100

typedef	struct count_peak_config {
  int window_size;
  int peak_tempo;
  float threshold;
} count_peak_config_t;

extern void detect_peak(int k, float sig[3], float *peak);

extern void detect_peak_2(int k, float sig[3], float *peak);

extern void peak_setparam(count_peak_config_t peak_params);

#endif
