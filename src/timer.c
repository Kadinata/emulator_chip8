#include <stdint.h>
#include <sys/time.h>
#include "timer.h"
#include "status_code.h"

#define USEC_PER_SEC (1000000)

/**
 * Helper function to calculate the amount of time that has elapsed between 
 * two timestamps and return the difference in microseconds.
 */
static inline uint64_t time_diff(struct timeval * const start, struct timeval * const end)
{
  return (uint64_t)((end->tv_sec - start->tv_sec) * USEC_PER_SEC) + (uint64_t)(end->tv_usec - start->tv_usec);
}

status_code_t timer_init(timer_t *const timer, uint32_t const timer_freq_hz)
{
  VERIFY_PTR_RETURN_ERROR_IF_NULL(timer);

  if (timer_freq_hz <= 0) {
    return STATUS_ERR_MATH_DIV_0;
  }

  timer->period_us = 1000000 / timer_freq_hz;
  gettimeofday(&timer->timestamp, NULL);

  return STATUS_OK;
}

uint8_t timer_check(timer_t *const timer) 
{
  struct timeval current;
  gettimeofday(&current, NULL);

  if (time_diff(&timer->timestamp, &current) >= timer->period_us) {
    gettimeofday(&timer->timestamp, NULL);
    return 1;
  }

  return 0;
}
