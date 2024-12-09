#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include <sys/time.h>
#include "status_code.h"

/** Timer data structure definitions */
typedef struct timer_s
{
  /** Period of this timer in microseconds */
  uint64_t period_us;

  /** Timestamp of the last tick of this timer */
  struct timeval timestamp;
} timer_t;

/**
 * Initialize a timer instance and sets its frequency.
 * @param timer - Pointer to the timer object to initialize.
 * @param timer_freq_hz - Desired frequency for the timer.
 * @return STATUS_OK if initialization is successful, otherwise appropriate error code.
 */
status_code_t timer_init(timer_t *const timer, uint32_t const timer_freq_hz);

/**
 * Check whether the amount of elapsed time has exceeded the timer's period
 * since the last tick of the timer. This function should be called
 * in the main loop as often as possible to ensure the timer runs as close as
 * possible to its desired frequency.
 * @param timer - Pointer to the timer object to check.
 * @return 1 if the amount of time between now and the timer's last tick has
 *         exceeded the timer's period, 0 otherwise.
 */
uint8_t timer_check(timer_t *const timer);

#endif /* __TIMER_H__ */
