#ifndef __CHIP_8_H__
#define __CHIP_8_H__

#include "cpu_def.h"
#include "status_code.h"

status_code_t init_cpu(cpu_state_t *state);
status_code_t load_rom(cpu_state_t *state, const char *file);
status_code_t emulation_cycle(cpu_state_t *state);

#endif