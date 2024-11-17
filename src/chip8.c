#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "chip8.h"
#include "cpu_def.h"
#include "status_code.h"

status_code_t fetch(cpu_state_t *const state, uint16_t *const opcode);
status_code_t mem_read(cpu_state_t *const state, const uint16_t address, uint8_t *const dest, const size_t size);
status_code_t mem_write(cpu_state_t *const state, const uint16_t address, uint8_t *const source, const size_t size);

status_code_t op_table_0(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_table_8(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_table_E(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_table_F(uint16_t const opcode, cpu_state_t *const state);

status_code_t op_00E0(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_00EE(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_1NNN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_2NNN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_3XNN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_4XNN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_5XY0(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_6XNN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_7XNN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_8XY0(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_8XY1(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_8XY2(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_8XY3(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_8XY4(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_8XY5(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_8X06(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_8XY7(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_8X0E(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_9XY0(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_ANNN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_BNNN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_CXNN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_DXYN(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_EX9E(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_EXA1(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_FX07(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_FX0A(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_FX15(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_FX18(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_FX1E(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_FX29(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_FX33(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_FX55(uint16_t const opcode, cpu_state_t *const state);
status_code_t op_FX65(uint16_t const opcode, cpu_state_t *const state);

typedef status_code_t (*opcode_handler_fn)(uint16_t const opcode, cpu_state_t *const state);

uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

status_code_t init_cpu(cpu_state_t *const state)
{
  VERIFY_PTR_RETURN_ERROR_IF_NULL(state);

  srand((uint16_t)time(NULL));
  memset(state, 0, sizeof(cpu_state_t));
  memcpy(state->memory, fontset, sizeof(fontset));
  state->registers.pc = START_ADDRESS;

  return STATUS_OK;
}

status_code_t load_rom(cpu_state_t *const state, const char *file)
{
  VERIFY_PTR_RETURN_ERROR_IF_NULL(state);
  VERIFY_PTR_RETURN_ERROR_IF_NULL(file);

  FILE *fp = fopen(file, "rb");

  if (fp == NULL)
  {
    return STATUS_ERR_FILE_NOT_FOUND;
  }

  struct stat st;
  stat(file, &st);
  size_t fsize = st.st_size;

  size_t bytes_read = fread(state->memory + START_ADDRESS, 1, sizeof(state->memory) - START_ADDRESS, fp);

  fclose(fp);

  if (bytes_read != fsize)
  {
    return STATUS_ERR_NO_MEMORY;
  }

  return STATUS_OK;
}

status_code_t emulation_cycle(cpu_state_t *const state)
{
  // Instruction table
  static opcode_handler_fn op_table[16] = {
      op_table_0, op_1NNN, op_2NNN, op_3XNN,
      op_4XNN, op_5XY0, op_6XNN, op_7XNN,
      op_table_8, op_9XY0, op_ANNN, op_BNNN,
      op_CXNN, op_DXYN, op_table_E, op_table_F};

  VERIFY_PTR_RETURN_ERROR_IF_NULL(state);

  uint16_t opcode;
  status_code_t status;

  // Fetch
  status = fetch(state, &opcode);
  RETURN_STATUS_IF_NOT_OK(status);

  // Decode + Execute
  status = op_table[((opcode >> 12) & 0xF)](opcode, state);
  RETURN_STATUS_IF_NOT_OK(status);

  // Decrement timers
  if (state->timers.delay > 0)
  {
    state->timers.delay--;
  }

  if (state->timers.sound > 0)
  {
    state->timers.sound--;
  }

  return STATUS_OK;
}

/** Private */
status_code_t fetch(cpu_state_t *const state, uint16_t *const opcode)
{
  status_code_t status = STATUS_OK;
  registers_t *reg = &state->registers;

  /** Instructions are 2 bytes. PC must always be pointing to an even address */
  if (reg->pc & 0x1)
  {
    return STATUS_ERR_INVALID_ADDRESS;
  }

  uint8_t bytes[2] = {0};
  status = mem_read(state, reg->pc, bytes, 2);
  RETURN_STATUS_IF_NOT_OK(status);

  *opcode = (uint16_t)((bytes[0] << 8)) | bytes[1];
  reg->pc += 2;

  return STATUS_OK;
}

status_code_t mem_read(cpu_state_t *const state, const uint16_t address, uint8_t *const dest, const size_t size)
{
  if ((address + size - 1) >= MEM_SIZE)
  {
    return STATUS_ERR_MEM_OUT_OF_BOUNDS;
  }

  memcpy(dest, &(state->memory[address]), size);
  return STATUS_OK;
}

status_code_t mem_write(cpu_state_t *const state, const uint16_t address, uint8_t *const source, const size_t size)
{
  if ((address + size - 1) >= MEM_SIZE)
  {
    return STATUS_ERR_MEM_OUT_OF_BOUNDS;
  }

  memcpy(&(state->memory[address]), source, size);
  return STATUS_OK;
}

status_code_t op_table_0(uint16_t const opcode, cpu_state_t *const state)
{
  status_code_t status = STATUS_OK;

  switch (opcode & 0x00FF)
  {
  case 0xE0:
    status = op_00E0(opcode, state);
    break;
  case 0xEE:
    status = op_00EE(opcode, state);
    break;
  default:
    break;
  }

  return status;
}

status_code_t op_table_8(uint16_t const opcode, cpu_state_t *const state)
{
  status_code_t status = STATUS_OK;

  switch (opcode & 0x000F)
  {
  case 0x0:
    status = op_8XY0(opcode, state);
    break;
  case 0x1:
    status = op_8XY1(opcode, state);
    break;
  case 0x2:
    status = op_8XY2(opcode, state);
    break;
  case 0x3:
    status = op_8XY3(opcode, state);
    break;
  case 0x4:
    status = op_8XY4(opcode, state);
    break;
  case 0x5:
    status = op_8XY5(opcode, state);
    break;
  case 0x6:
    status = op_8X06(opcode, state);
    break;
  case 0x7:
    status = op_8XY7(opcode, state);
    break;
  case 0xE:
    status = op_8X0E(opcode, state);
    break;
  default:
    break;
  }
  return status;
}

status_code_t op_table_E(uint16_t const opcode, cpu_state_t *const state)
{
  status_code_t status = STATUS_OK;

  switch (opcode & 0xFF)
  {
  case 0x9E:
    status = op_EX9E(opcode, state);
    break;
  case 0xA1:
    status = op_EXA1(opcode, state);
    break;
  default:
    break;
  }
  return status;
}

status_code_t op_table_F(uint16_t const opcode, cpu_state_t *const state)
{
  status_code_t status = STATUS_OK;

  switch (opcode & 0xFF)
  {
  case 0x07:
    status = op_FX07(opcode, state);
    break;
  case 0x0A:
    status = op_FX0A(opcode, state);
    break;
  case 0x15:
    status = op_FX15(opcode, state);
    break;
  case 0x18:
    status = op_FX18(opcode, state);
    break;
  case 0x1E:
    status = op_FX1E(opcode, state);
    break;
  case 0x29:
    status = op_FX29(opcode, state);
    break;
  case 0x33:
    status = op_FX33(opcode, state);
    break;
  case 0x55:
    status = op_FX55(opcode, state);
    break;
  case 0x65:
    status = op_FX65(opcode, state);
    break;
  default:
    break;
  }

  return status;
}

/**
 * 0x00E0: CLS
 * Clears the screen
 */
status_code_t op_00E0(uint16_t const opcode, cpu_state_t *const state)
{
  memset(state->peripherals.graphics, 0, GRAPHICS_SIZE);
  return STATUS_OK;
}

/**
 * 0x00EE: RET
 * Return from a subroutine call
 */
status_code_t op_00EE(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  if (reg->sp == 0)
  {
    return STATUS_ERR_STACK_UNDERFLOW;
  }

  reg->sp--;
  reg->pc = reg->stack[reg->sp];

  return STATUS_OK;
}

/**
 * 0x1NNN: JMP NNN
 * Jump to address 0xNNN
 */
status_code_t op_1NNN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;
  reg->pc = opcode & 0x0FFF;

  return STATUS_OK;
}

/**
 * 0x2NNN: JSR NNN
 * Jump to subroutine at address 0xNNN
 */
status_code_t op_2NNN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  if (reg->sp >= STACK_SIZE)
  {
    return STATUS_ERR_STACK_OVERFLOW;
  }

  reg->stack[reg->sp] = reg->pc;
  reg->sp++;
  reg->pc = opcode & 0x0FFF;

  return STATUS_OK;
}

/**
 * 0x3XNN: SKEQ Vx, NN
 * Skip if V[X] == 0xNN
 */
status_code_t op_3XNN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  if (reg->V[x] == (opcode & 0x00FF))
  {
    reg->pc += 2;
  }

  return STATUS_OK;
}

/**
 * 0x4XNN: SKNE Vx, NN
 * Skip if V[X] != 0xNN
 */
status_code_t op_4XNN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  if (reg->V[x] != (opcode & 0x00FF))
  {
    reg->pc += 2;
  }

  return STATUS_OK;
}

/**
 * 0x5XY0: SKEQ Vx, Vy
 * Skip if V[X] == V[Y]
 */
status_code_t op_5XY0(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  if (reg->V[x] == reg->V[y])
  {
    reg->pc += 2;
  }

  return STATUS_OK;
}

/**
 * 0x6XNN: MOV Vx, NN
 * Sets the value of register V[X] to 0xNN
 */
status_code_t op_6XNN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  reg->V[x] = (opcode & 0x00FF);

  return STATUS_OK;
}

/**
 * 0x7XNN: ADD Vx, NN
 * Adds 0xNN to register V[X]
 * Note: Does not change the carry flag
 */
status_code_t op_7XNN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  reg->V[x] += (opcode & 0x00FF);

  return STATUS_OK;
}

/**
 * 0x8XY0: MOV Vx, Vy
 * Sets V[X] to the value of V[Y]
 */
status_code_t op_8XY0(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  reg->V[x] = reg->V[y];

  return STATUS_OK;
}

/**
 * 0x8XY1: OR Vx, Vy
 * Bitwise-OR the value in register V[Y] into V[X]
 */
status_code_t op_8XY1(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  reg->V[x] |= reg->V[y];

  return STATUS_OK;
}

/**
 * 0x8XY2: AND Vx, Vy
 * Bitwise-AND the value in register V[Y] into V[X]
 */
status_code_t op_8XY2(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  reg->V[x] &= reg->V[y];

  return STATUS_OK;
}

/**
 * 0x8XY3: XOR Vx, Vy
 * Bitwise-XOR the value in register V[Y] into V[X]
 */
status_code_t op_8XY3(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  reg->V[x] ^= reg->V[y];

  return STATUS_OK;
}

/**
 * 0x8XY4: ADD Vx, Vy
 * Adds the value in register V[Y] into V[X]
 * Note: Carry is stored in V[F]
 */
status_code_t op_8XY4(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  uint16_t sum = 0;

  sum = reg->V[x];
  sum += reg->V[y];

  reg->V[0xF] = (sum & 0xFF00) ? 1 : 0;
  reg->V[x] = (sum & 0x00FF);

  return STATUS_OK;
}

/**
 * 0x8XY5: SUB Vx, Vy
 * Sets V[X] to V[X] minus V[Y].
 * Note: V[F] is set to 0 when there's an underflow, and 1 when there isn't.
 */
status_code_t op_8XY5(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  reg->V[0xF] = (reg->V[x] > reg->V[y]) ? 1 : 0;
  reg->V[x] -= reg->V[y];

  return STATUS_OK;
}

/**
 * 0x8X06: SHR Vx
 * Shifts V[X] to the right by 1 and stores the LSB prior to the shift in V[F]
 */
status_code_t op_8X06(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  reg->V[0xF] = (reg->V[x] & 0x1);
  reg->V[x] >>= 1;

  return STATUS_OK;
}

/**
 * 0x8XY7: RSUB Vx, Vy
 * Sets V[X] to V[Y] minus V[X].
 * Note: V[F] is set to 0 when there's an underflow, and 1 when there isn't
 */
status_code_t op_8XY7(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  reg->V[0xF] = (reg->V[y] > reg->V[x]) ? 1 : 0;
  reg->V[x] = reg->V[y] - reg->V[x];

  return STATUS_OK;
}

/**
 * 0x8X0E: SHL Vx
 * Shifts V[X] to the left by 1 and stores the MSB prior to the shift in V[F]
 */
status_code_t op_8X0E(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  reg->V[0xF] = (reg->V[x] & 0x80) >> 7;
  reg->V[x] <<= 1;

  return STATUS_OK;
}

/**
 * 0x9XY0: SKNE Vx, Vy
 * Skip if V[X] != V[Y]
 */
status_code_t op_9XY0(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;

  if (reg->V[x] != reg->V[y])
  {
    reg->pc += 2;
  }

  return STATUS_OK;
}

/**
 * 0xANNN: MVI NNN
 * Sets the index register to 0xNNN
 */
status_code_t op_ANNN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  reg->I = (opcode & 0xFFF);

  return STATUS_OK;
}

/**
 * 0xBNNN: JMI NNN
 * Jump to the address 0xNNN + V0
 */
status_code_t op_BNNN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  reg->pc = reg->V[0] + (opcode & 0x0FFF);

  return STATUS_OK;
}

/**
 * 0xCXNN: RAND Vx, NN
 * Sets V[X] to a random number bitwise-and'ed with 0xNN
 */
status_code_t op_CXNN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  reg->V[x] = (rand() & (opcode)) & 0x00FF;

  return STATUS_OK;
}

/**
 * 0xDXYN: DISP X, Y, N
 * Draw a sprite on the display at (V[X], V[Y]) that has a height of N
 */
status_code_t op_DXYN(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  uint8_t h = (opcode & 0x000F);
  uint8_t pixel = 0;

  uint16_t x_pos = reg->V[x] % GRAPHICS_WIDTH;
  uint16_t y_pos = reg->V[y] % GRAPHICS_HEIGHT;

  reg->V[0xF] = 0;
  for (uint16_t row = 0; row < h; row++)
  {
    pixel = state->memory[reg->I + row];

    for (uint16_t col = 0; col < 8; col++)
    {
      if (pixel & (0x80 >> col))
      {
        uint16_t screen_pixel_index = (x_pos + col) + ((y_pos + row) * GRAPHICS_WIDTH);

        if (state->peripherals.graphics[screen_pixel_index])
        {
          reg->V[0xF] = 1;
        }

        state->peripherals.graphics[screen_pixel_index] ^= 1;
      }
    }
  }

  return STATUS_OK;
}

/**
 * 0xEX9E: SKPR Vx
 * Skip if the key stored in V[X] is pressed
 */
status_code_t op_EX9E(uint16_t const opcode, cpu_state_t *const state)
{
  uint8_t x = (opcode & 0x0F00) >> 8;

  registers_t *reg = &state->registers;

  if (state->peripherals.keypad[reg->V[x]])
  {
    reg->pc += 2;
  }

  return STATUS_OK;
}

/**
 * 0xEXA1: SKNP Vx
 * Skip if the key stored in V[X] is not pressed
 */
status_code_t op_EXA1(uint16_t const opcode, cpu_state_t *const state)
{
  uint8_t x = (opcode & 0x0F00) >> 8;

  registers_t *reg = &state->registers;

  if (!state->peripherals.keypad[reg->V[x]])
  {
    reg->pc += 2;
  }

  return STATUS_OK;
}

/**
 * 0xFX07: GDLY Vx
 * Sets V[X] to the value of the delay timer
 */
status_code_t op_FX07(uint16_t const opcode, cpu_state_t *const state)
{
  uint8_t x = (opcode & 0x0F00) >> 8;

  registers_t *reg = &state->registers;

  reg->V[x] = state->timers.delay;

  return STATUS_OK;
}

/**
 * 0xFX0A: KEY Vx
 * Wait for key press and stores the value in V[X]
 */
status_code_t op_FX0A(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t key_pressed = 0;
  uint8_t x = (opcode & 0x0F00) >> 8;

  for (uint8_t i = 0; i < NUM_KEYS; i++)
  {
    if (state->peripherals.keypad[i])
    {
      key_pressed = 1;
      reg->V[x] = i;
    }
  }

  if (!key_pressed)
  {
    reg->pc -= 2;
  }

  return STATUS_OK;
}

/**
 * 0xFX15: SDLY Vx
 * Set the delay timer to the value stored in V[X]
 */
status_code_t op_FX15(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  state->timers.delay = reg->V[x];

  return STATUS_OK;
}

/**
 * 0xFX18: SSND Vx
 * Set the sound timer to the value stored in V[X]
 */
status_code_t op_FX18(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  state->timers.sound = reg->V[x];

  return STATUS_OK;
}

/**
 * 0xFX1E: ADI Vx
 * Add register V[X] to the index register
 */
status_code_t op_FX1E(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  reg->I += reg->V[x];

  return STATUS_OK;
}

/**
 * 0xFX29: FONT Vx
 * Sets I to the location of the sprite for the character in V[X]
 */
status_code_t op_FX29(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;

  reg->I = reg->V[x] * 5;

  return STATUS_OK;
}

/**
 * 0xFX33: BCD Vx
 * Stores the binary-coded decimal representation of V[X],
 * with the hundreds digit in memory at location in I,
 * the tens digit at location I+1,
 * and the ones digit at location I+2.
 * Note: This doesn't change I
 */
status_code_t op_FX33(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t value = reg->V[x];
  uint8_t bcd[3] = {0};

  bcd[2] = value % 10;
  value /= 10;

  bcd[1] = value % 10;
  value /= 10;

  bcd[0] = value % 10;

  return mem_write(state, reg->I, bcd, 3);
}

/**
 * 0xFX55: STR V0, Vx
 * Stores the values in V[0] to V[X] (including V[X]) in memory, starting at address I.
 * Note: This doesn't change I
 */
status_code_t op_FX55(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  size_t size = ((opcode & 0x0F00) >> 8) + 1;

  return mem_write(state, reg->I, reg->V, size);
}

/**
 * 0xFX65: LDR V0, Vx
 * Fills from V[0] to V[X] (including V[X]) with values from memory, starting at address I.
 * Note: This doesn't change I
 */
status_code_t op_FX65(uint16_t const opcode, cpu_state_t *const state)
{
  registers_t *reg = &state->registers;

  size_t size = ((opcode & 0x0F00) >> 8) + 1;

  return mem_read(state, reg->I, reg->V, size);
}