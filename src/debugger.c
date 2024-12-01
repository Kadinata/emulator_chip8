#include <stdio.h>

#include "cpu_def.h"
#include "debugger.h"
#include "status_code.h"

status_code_t debug_cpu_state(cpu_state_t *const state)
{

  VERIFY_PTR_RETURN_ERROR_IF_NULL(state);

#ifdef DEBUG
  uint16_t opcode = state->memory[state->registers.pc] << 8 | state->memory[state->registers.pc + 1];

  fprintf(stderr, "\033[2J\033[H");
  fprintf(stderr, "PC: 0x%04X (0x%04X)\n", state->registers.pc, opcode);
  fprintf(stderr, "I : 0x%04X     ", state->registers.I);
  fprintf(stderr, "SP: 0x%04X\n\n", state->registers.sp);

  for (int8_t i = 0; i < REG_COUNT; i++)
  {
    fprintf(stderr, "V[%2d]: 0x%02X    ", i, state->registers.V[i]);
    fprintf(stderr, "Stack[%2d]: 0x%04X    ", i, state->registers.stack[i]);
    fprintf(stderr, "Key[%2d]: %d\n", i, state->peripherals.keypad[i]);
  }
#endif

  return STATUS_OK;
}
