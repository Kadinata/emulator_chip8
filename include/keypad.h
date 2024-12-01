#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <stdint.h>
#include "status_code.h"

/**
 * Read the keypad state and store the results in the provided buffer.
 * @param key_buf - Pointer to the buffer to store keypad readings. Value
 *                  is set to 1 if the key is pressed, 0 otherwise.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t keypad_read(uint8_t * const key_buf);

#endif /* __KEYPAD_H__ */
