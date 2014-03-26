#ifndef IOTLAB_TIME_H
#define IOTLAB_TIME_H



// "Implememnation assumes SOFT_TIMER_FREQUENCY == 32768"

extern struct soft_timer_timeval time0;

/* Sets current time as reference in time0 */
void iotlab_time_reset_time(void);

/*
 * Extend given timer_tick in ticks to a struct soft_timer_timeval
 * and set it relative to last reset_time
 */
void iotlab_time_extend_relative(struct soft_timer_timeval *extended_time,
        uint32_t timer_tick);

#endif // IOTLAB_TIME_H
