#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>
#include <sys/time.h>
#include "status_code.h"

typedef struct timer_s
{
  uint64_t period_us;
  struct timeval start;
  struct timeval end;
} timer_t;

status_code_t timer_init(timer_t *const timer, uint32_t timer_freq_hz);
uint8_t timer_check(timer_t *const timer);

#endif /* __TIMER_H__ */
