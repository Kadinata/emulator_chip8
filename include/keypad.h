#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <stdint.h>
#include "status_code.h"

status_code_t keypad_read(uint8_t * const key_buf);

#endif
