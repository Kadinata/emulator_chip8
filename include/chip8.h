#ifndef __CHIP_8_H__
#define __CHIP_8_H__

#include "cpu_def.h"

void init_cpu(cpu_state_t *state);
int32_t load_rom(cpu_state_t *state, const char *file);
void emulation_cycle(cpu_state_t *state);

#endif