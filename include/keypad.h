#ifndef __KEYPAD_H__
#define __KEYPAD_H__

#include <stdint.h>
#include "cpu_def.h"
#include "status_code.h"

/**
 * Read the keypad state and store the results in the provided keypad state object.
 * @param keypad - Pointer to keypad state object to store keypad reading values.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t keypad_read(keypad_state_t *const keypad);

#endif /* __KEYPAD_H__ */
