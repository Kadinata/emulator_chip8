#ifndef __CHIP_8_H__
#define __CHIP_8_H__

#include "cpu_def.h"
#include "status_code.h"

/**
 * Initialize the provided CPU state by setting the value of PC to the
 * starting address and the memory to 0.
 * @param state - Pointer to the CPU state to initialize
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t init_cpu(cpu_state_t *const state);

/**
 * Load a ROM file to the memory.
 * @param state - Pointer to a CPU state onto which the rom file will be loaded
 * @param file - Path to the .ch8 rom file
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t load_rom(cpu_state_t *const state, const char *file);

/**
 * Executes a single CPU cycle (fetch, decode, and execute);
 * also decrement timers if their values are not 0.
 * @param state - Pointer to a CPU state.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t emulation_cycle(cpu_state_t *const state);

/**
 * Decrement the display and sound timers if > 0.
 * This should be called at 60 Hz rate.
 * @param state - Pointer to a CPU state.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t update_timers(cpu_state_t *const state);

#endif /* __CHIP_8_H__ */
