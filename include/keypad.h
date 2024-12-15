#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <stdint.h>
#include "status_code.h"

/**
 * Read the keypad state and store the results in the provided flags.
 * @param key_state - Pointer to a 16-bit flags to store keypad reading values.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t keypad_read(uint16_t *const key_state);

#endif /* __KEYPAD_H__ */
