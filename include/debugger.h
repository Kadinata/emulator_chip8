#ifndef __CHIP_8_DEBUGGER_H__
#define __CHIP_8_DEBUGGER_H__

#include "cpu_def.h"
#include "status_code.h"

status_code_t debug_cpu_state(cpu_state_t *const state);

#endif /* __CHIP_8_DEBUGGER_H__ */
