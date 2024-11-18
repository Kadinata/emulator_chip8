#ifndef __CHIP_8_CPU_DEF_H__
#define __CHIP_8_CPU_DEF_H__

#include <stdint.h>

#define START_ADDRESS (0x0200)
#define MEM_SIZE (4096) // 4K RAM
#define GRAPHICS_WIDTH (64)
#define GRAPHICS_HEIGHT (32)
#define GRAPHICS_SIZE (GRAPHICS_WIDTH * GRAPHICS_HEIGHT)
#define NUM_KEYS (16)
#define STACK_SIZE (16)
#define REG_COUNT (16)

/** Definitions of CPU registers */
typedef struct registers_s
{
  /** 8-bit stack pointer */
  uint8_t sp;

  /** 16-bit program counter */
  uint16_t pc;

  /** 16-bit index register; a special register to store memory address */
  uint16_t I;

  /** 16 x 8-bit general purpose registers V0-VF */
  uint8_t V[REG_COUNT];

  /** 16 x 16-bit stack to store return addresses when subroutines are called */
  uint16_t stack[STACK_SIZE];
} registers_t;

/**
 * Definitions of CPU timers.
 * Each one decerements by one at each cycle if its value is greater than 0.
 */
typedef struct timers_s
{
  uint8_t delay;
  uint8_t sound;
} timers_t;

/** Display / graphics */
typedef struct graphics_s
{
  /** buffer for 64x32 px output monochrome display */
  uint8_t buffer[GRAPHICS_SIZE];

  /** Flag indicating the screen needs to be updated */
  uint8_t display_update;
} graphics_t;

/** Definitions of CPU I/O / peripherals */
typedef struct peripherals_s
{
  /** 16 x 8-bit input keypad */
  uint8_t keypad[NUM_KEYS];

  /** 64x32 px output monochrome display */
  graphics_t graphics;
} peripherals_t;

/** CPU state definitions */
typedef struct cpu_state_s
{
  uint8_t memory[MEM_SIZE];
  registers_t registers;
  timers_t timers;
  peripherals_t peripherals;
} cpu_state_t;

#endif
