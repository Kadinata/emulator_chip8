#include <stdint.h>
#include <sys/time.h>
#include "timer.h"
#include "status_code.h"

static inline uint64_t time_diff(struct timeval * const start, struct timeval * const end)
{
  return (uint64_t)((end->tv_sec - start->tv_sec) * 1000000) + (uint64_t)(end->tv_usec - start->tv_usec);
}

status_code_t timer_init(timer_t *const timer, uint32_t timer_freq_hz)
{
  VERIFY_PTR_RETURN_ERROR_IF_NULL(timer);

  if (timer_freq_hz <= 0) {
    return STATUS_ERR_MATH_DIV_0;
  }

  timer->period_us = 1000000 / timer_freq_hz;
  gettimeofday(&timer->start, NULL);

  return STATUS_OK;
}

uint8_t timer_check(timer_t *const timer) 
{
  gettimeofday(&timer->end, NULL);

  if (time_diff(&timer->start, &timer->end) >= timer->period_us) {
    gettimeofday(&timer->start, NULL);
    return 1;
  }

  return 0;
}
