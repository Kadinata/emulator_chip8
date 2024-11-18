#include "unity.h"
#include "chip8.h"
#include "cpu_def.h"
#include "status_code.h"
#include "string.h"

TEST_FILE("chip8.c")

void setUp(void)
{
}

void tearDown(void)
{
}

void stub_init_cpu_state(cpu_state_t *cpu_state)
{
  cpu_state->registers.pc = START_ADDRESS;
}

void stub_set_opcode(cpu_state_t *cpu_state, uint16_t opcode, uint16_t offset)
{
  cpu_state->memory[START_ADDRESS + offset] = (uint8_t)((opcode & 0xFF00) >> 8);
  cpu_state->memory[START_ADDRESS + offset + 1] = (uint8_t)((opcode & 0x00FF));
}

void stub_clear_mem(cpu_state_t *cpu_state)
{
  for (int16_t i = 0; i < MEM_SIZE; i++)
  {
    cpu_state->memory[i] = 0;
  }
}

void stub_clear_V(cpu_state_t *cpu_state)
{
  for (int8_t i = 0; i < REG_COUNT; i++)
  {
    cpu_state->registers.V[i] = 0;
  }
}

void test_init_cpu(void)
{
  cpu_state_t cpu_state;
  memset(&cpu_state, 0xAA, sizeof(cpu_state_t));

  TEST_ASSERT_EQUAL_INT(STATUS_OK, init_cpu(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS, cpu_state.registers.pc);
  TEST_ASSERT_EQUAL_HEX8(0, cpu_state.registers.sp);
  TEST_ASSERT_EQUAL_HEX16(0, cpu_state.registers.I);

  for (int8_t i = 0; i < REG_COUNT; i++)
  {
    TEST_ASSERT_EQUAL_HEX8(0, cpu_state.registers.V[i]);
  }
}

void test_init_cpu_with_null_ptr(void)
{
  TEST_ASSERT_EQUAL_INT(STATUS_ERR_NULL_PTR, init_cpu(NULL));
}

void test_emulation_cycle_fetch_with_valid_address(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xFFF0, 0);

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 2, cpu_state.registers.pc);
}

void test_emulation_cycle_fetch_with_invalid_address(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xFFFF, 0);
  cpu_state.registers.pc += 1;

  TEST_ASSERT_EQUAL_INT(STATUS_ERR_INVALID_ADDRESS, emulation_cycle(&cpu_state));
}

void test_emulation_cycle_fetch_with_out_of_bound_address(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  cpu_state.registers.pc = 0x1000;

  TEST_ASSERT_EQUAL_INT(STATUS_ERR_MEM_OUT_OF_BOUNDS, emulation_cycle(&cpu_state));
}

void test_emulation_cycle_decrement_timers(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xFFF0, 0);
  cpu_state.timers.delay = 0xAA;
  cpu_state.timers.sound = 0x55;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(0xAA - 1, cpu_state.timers.delay);
  TEST_ASSERT_EQUAL_HEX16(0x55 - 1, cpu_state.timers.sound);
}

void test_emulation_cycle_timers_at_0(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xFFF0, 0);
  cpu_state.timers.delay = 0;
  cpu_state.timers.sound = 0;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(0, cpu_state.timers.delay);
  TEST_ASSERT_EQUAL_HEX16(0, cpu_state.timers.sound);
}

void test_emulation_cycle_NOPs(void)
{
  cpu_state_t cpu_state = {0};
  uint8_t opcodes[8] = {
      0x00, 0x00,
      0x80, 0x0A,
      0xE0, 0x00,
      0xFF, 0xFF};
  stub_init_cpu_state(&cpu_state);
  memcpy(&cpu_state.memory[START_ADDRESS], opcodes, sizeof(opcodes));

  for (int8_t i = 0; i < 4; i++)
  {
    TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  }
}

void test_emulation_cycle_null_ptr_check(void)
{
  TEST_ASSERT_EQUAL_INT(STATUS_ERR_NULL_PTR, emulation_cycle(NULL));
}

/**
 * Test 0x00E0: CLS
 * Clears the screen
 */
void test_op_00E0(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x00E0, 0);

  cpu_state.peripherals.graphics.display_update = 0;
  for (int16_t i = 0; i < GRAPHICS_SIZE; i++)
  {
    cpu_state.peripherals.graphics.buffer[i] = (i % 2) ? 0x55 : 0xAA;
  }

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  for (int16_t i = 0; i < GRAPHICS_SIZE; i++)
  {
    TEST_ASSERT_EQUAL_HEX8(0x00, cpu_state.peripherals.graphics.buffer[i]);
  }

  TEST_ASSERT_EQUAL_INT(1, cpu_state.peripherals.graphics.display_update);
}

/**
 * Test 0x00EE: RET
 * Return from a subroutine call
 */
void test_op_00EE_no_stack_underflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x00EE, 0);

  cpu_state.registers.stack[0] = 0x0AAA;
  cpu_state.registers.sp = 1;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(0x0AAA, cpu_state.registers.pc);
  TEST_ASSERT_EQUAL_HEX16(0, cpu_state.registers.sp);
}

/**
 * Test 0x00EE: RET
 * Return from a subroutine call
 */
void test_op_00EE_with_stack_underflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x00EE, 0);

  TEST_ASSERT_EQUAL_INT(STATUS_ERR_STACK_UNDERFLOW, emulation_cycle(&cpu_state));
}

/**
 * Test 0x1NNN: JMP NNN
 * Jump to address 0xNNN
 */
void test_op_1NNN(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x1A5A, 0);

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(0xA5A, cpu_state.registers.pc);
}

/**
 * Test 0x2NNN: JSR NNN
 * Jump to subroutine at address 0xNNN
 */
void test_op_2NNN_no_stack_overflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x2A5A, 0);

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(0xA5A, cpu_state.registers.pc);
  TEST_ASSERT_EQUAL_UINT8(1, cpu_state.registers.sp);
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 2, cpu_state.registers.stack[0]);
}

/**
 * Test 0x2NNN: JSR NNN
 * Jump to subroutine at address 0xNNN
 */
void test_op_2NNN_with_stack_overflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x2A5A, 0);

  for (int8_t i = 0; i < STACK_SIZE; i++)
  {
    TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
    stub_init_cpu_state(&cpu_state);
  }

  TEST_ASSERT_EQUAL_INT(STATUS_ERR_STACK_OVERFLOW, emulation_cycle(&cpu_state));
}

/**
 * Test 0x3XNN: SKEQ Vx, NN
 * Skip if V[X] == 0xNN
 */
void test_op_3NNN_Vx_equal_NN(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x30AA, 0);
  cpu_state.registers.V[0] = 0xAA;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 4, cpu_state.registers.pc);
}

/**
 * Test 0x3XNN: SKEQ Vx, NN
 * Skip if V[X] == 0xNN
 */
void test_op_3NNN_Vx_not_equal_NN(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x30AA, 0);
  cpu_state.registers.V[0] = 0x55;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 2, cpu_state.registers.pc);
}

/**
 * Test 0x4XNN: SKNE Vx, NN
 * Skip if V[X] != 0xNN
 */
void test_op_4NNN_Vx_equal_NN(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x40AA, 0);
  cpu_state.registers.V[0] = 0xAA;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 2, cpu_state.registers.pc);
}

/**
 * Test 0x4XNN: SKNE Vx, NN
 * Skip if V[X] != 0xNN
 */
void test_op_4NNN_Vx_not_equal_NN(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x40AA, 0);
  cpu_state.registers.V[0] = 0x55;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 4, cpu_state.registers.pc);
}

/**
 * Test 0x5XY0: SKEQ Vx, Vy
 * Skip if V[X] == V[Y]
 */
void test_op_5XY0_Vx_equal_Vy(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x5010, 0);
  cpu_state.registers.V[0] = 0xAA;
  cpu_state.registers.V[1] = 0xAA;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 4, cpu_state.registers.pc);
}

/**
 * Test 0x5XY0: SKEQ Vx, Vy
 * Skip if V[X] == V[Y]
 */
void test_op_5XY0_Vx_not_equal_Vy(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x5010, 0);
  cpu_state.registers.V[0] = 0xAA;
  cpu_state.registers.V[1] = 0x55;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 2, cpu_state.registers.pc);
}

/**
 * Test 0x6XNN: MOV Vx, NN
 * Sets the value of register V[X] to 0xNN
 */
void test_op_6XNN(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x60AA, 0);

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0xAA, cpu_state.registers.V[0]);
}

/**
 * Test 0x7XNN: ADD Vx, NN
 * Adds 0xNN to register V[X]
 * Note: Does not change the carry flag
 */
void test_op_7XNN_no_overflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x7056, 0);
  cpu_state.registers.V[0] = 0x24;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8((0x24 + 0x56), cpu_state.registers.V[0]);
}

/**
 * Test 0x7XNN: ADD Vx, NN
 * Adds 0xNN to register V[X]
 * Note: Does not change the carry flag
 */
void test_op_7XNN_with_overflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x7001, 0);
  cpu_state.registers.V[0] = 0xFF;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0x00, cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX8(0x00, cpu_state.registers.V[0xF]);
}

/**
 * Test 0x8XY0: MOV Vx, Vy
 * Sets V[X] to the value of V[Y]
 */
void test_op_8XY0(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8010, 0);
  cpu_state.registers.V[0] = 0x55;
  cpu_state.registers.V[1] = 0xAA;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0xAA, cpu_state.registers.V[0]);
}

/**
 * Test 0x8XY1: OR Vx, Vy
 * Bitwise-OR the value in register V[Y] into V[X]
 */
void test_op_8XY1(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8011, 0);
  cpu_state.registers.V[0] = 0x55;
  cpu_state.registers.V[1] = 0x33;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0x77, cpu_state.registers.V[0]);
}

/**
 * Test 0x8XY2: AND Vx, Vy
 * Bitwise-AND the value in register V[Y] into V[X]
 */
void test_op_8XY2(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8012, 0);
  cpu_state.registers.V[0] = 0x55;
  cpu_state.registers.V[1] = 0x33;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0x11, cpu_state.registers.V[0]);
}

/**
 * Test 0x8XY3: XOR Vx, Vy
 * Bitwise-XOR the value in register V[Y] into V[X]
 */
void test_op_8XY3(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8013, 0);
  cpu_state.registers.V[0] = 0x55;
  cpu_state.registers.V[1] = 0x33;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0x66, cpu_state.registers.V[0]);
}

/**
 * Test 0x8XY4: ADD Vx, Vy
 * Adds the value in register V[Y] into V[X]
 * Note: Carry is stored in V[F]
 */
void test_op_8XY4_no_overrflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8014, 0);
  cpu_state.registers.V[0] = 0x23;
  cpu_state.registers.V[1] = 0x45;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8((0x23 + 0x45), cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX8(0x45, cpu_state.registers.V[1]);
  TEST_ASSERT_EQUAL_HEX8(0, cpu_state.registers.V[0xF]);
}

/**
 * Test 0x8XY4: ADD Vx, Vy
 * Adds the value in register V[Y] into V[X]
 * Note: Carry is stored in V[F]
 */
void test_op_8XY4_with_overrflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8014, 0);
  cpu_state.registers.V[0] = 0xFF;
  cpu_state.registers.V[1] = 0x01;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0, cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX8(1, cpu_state.registers.V[0xF]);
}

/**
 * Test 0x8XY5: SUB Vx, Vy
 * Sets V[X] to V[X] minus V[Y].
 * Note: V[F] is set to 0 when there's an underflow, and 1 when there isn't.
 */
void test_op_8XY5_no_underflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8015, 0);
  cpu_state.registers.V[0] = 0x54;
  cpu_state.registers.V[1] = 0x32;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8((0x54 - 0x32), cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX8(0x32, cpu_state.registers.V[1]);
  TEST_ASSERT_EQUAL_HEX8(1, cpu_state.registers.V[0xF]);
}

/**
 * Test 0x8XY5: SUB Vx, Vy
 * Sets V[X] to V[X] minus V[Y].
 * Note: V[F] is set to 0 when there's an underflow, and 1 when there isn't.
 */
void test_op_8XY5_with_underflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8015, 0);
  cpu_state.registers.V[0] = 0x00;
  cpu_state.registers.V[1] = 0x01;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0xFF, cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX8(0, cpu_state.registers.V[0xF]);
}

/**
 * Test 0x8X06: SHR Vx
 * Shifts V[X] to the right by 1 and stores the LSB prior to the shift in V[F]
 */
void test_op_8X06(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8006, 0);
  cpu_state.registers.V[0] = 0x55;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0x2A, cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX8(1, cpu_state.registers.V[0xF]);
}

/**
 * Test 0x8XY7: RSUB Vx, Vy
 * Sets V[X] to V[Y] minus V[X].
 * Note: V[F] is set to 0 when there's an underflow, and 1 when there isn't
 */
void test_op_8XY7_no_underflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8017, 0);
  cpu_state.registers.V[0] = 0x23;
  cpu_state.registers.V[1] = 0x45;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8((0x45 - 0x23), cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX8(0x45, cpu_state.registers.V[1]);
  TEST_ASSERT_EQUAL_HEX8(1, cpu_state.registers.V[0xF]);
}

/**
 * Test 0x8XY7: RSUB Vx, Vy
 * Sets V[X] to V[Y] minus V[X].
 * Note: V[F] is set to 0 when there's an underflow, and 1 when there isn't
 */
void test_op_8XY7_with_underflow(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x8017, 0);
  cpu_state.registers.V[0] = 0x01;
  cpu_state.registers.V[1] = 0x00;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0xFF, cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX8(0, cpu_state.registers.V[0xF]);
}

/**
 * Test 0x8X0E: SHL Vx
 * Shifts V[X] to the left by 1 and stores the MSB prior to the shift in V[F]
 */
void test_op_8X0E(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x800E, 0);
  cpu_state.registers.V[0] = 0xAA;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0x54, cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX8(1, cpu_state.registers.V[0xF]);
}

/**
 * Test 0x9XY0: SKNE Vx, Vy
 * Skip if V[X] != V[Y]
 */
void test_op_9XY0_Vx_equal_Vy(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x9010, 0);
  cpu_state.registers.V[0] = 0xAA;
  cpu_state.registers.V[1] = 0xAA;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 2, cpu_state.registers.pc);
}

/**
 * Test 0x9XY0: SKNE Vx, Vy
 * Skip if V[X] != V[Y]
 */
void test_op_9XY0_Vx_not_equal_Vy(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0x9010, 0);
  cpu_state.registers.V[0] = 0xAA;
  cpu_state.registers.V[1] = 0x55;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 4, cpu_state.registers.pc);
}

/**
 * Test 0xANNN: MVI NNN
 * Sets the index register to 0xNNN
 */
void test_op_ANNN(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xABCD, 0);

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(0xBCD, cpu_state.registers.I);
}

/**
 * Test 0xBNNN: JMI NNN
 * Jump to the address 0xNNN + V0
 */
void test_op_BNNN(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xBCDE, 0);
  cpu_state.registers.V[0] = 0x12;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16((0xCDE + 0x12), cpu_state.registers.pc);
}

/**
 * Test 0xCXNN: RAND Vx, NN
 * Sets V[X] to a random number bitwise-and'ed with 0xNN
 */
void test_op_CXNN(void)
{
  /** TODO: Implement */
}

/**
 * Test 0xDXYN: DISP X, Y, N
 * Draw a sprite on the display at (V[X], V[Y]) that has a height of N
 */
void test_op_DXYN(void)
{
  /** TODO: Implement */
}

/**
 * Test 0xEX9E: SKPR Vx
 * Skip if the key stored in V[X] is pressed
 */
void test_op_EX9E_pressed(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xE09E, 0);
  cpu_state.registers.V[0] = 0;
  cpu_state.peripherals.keypad[0] = 1;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 4, cpu_state.registers.pc);
}

/**
 * Test 0xEX9E: SKPR Vx
 * Skip if the key stored in V[X] is pressed
 */
void test_op_EX9E_not_pressed(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xE09E, 0);
  cpu_state.registers.V[0] = 0;
  cpu_state.peripherals.keypad[0] = 0;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 2, cpu_state.registers.pc);
}

/**
 * Test 0xEXA1: SKNP Vx
 * Skip if the key stored in V[X] is not pressed
 */
void test_op_EXA1_pressed(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xE0A1, 0);
  cpu_state.registers.V[0] = 0;
  cpu_state.peripherals.keypad[0] = 1;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 2, cpu_state.registers.pc);
}

/**
 * Test 0xEXA1: SKNP Vx
 * Skip if the key stored in V[X] is not pressed
 */
void test_op_EXA1_not_pressed(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xE0A1, 0);
  cpu_state.registers.V[0] = 0;
  cpu_state.peripherals.keypad[0] = 0;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 4, cpu_state.registers.pc);
}

/**
 * Test 0xFX07: GDLY Vx
 * Sets V[X] to the value of the delay timer
 */
void test_op_FX07(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF007, 0);
  cpu_state.timers.delay = 0x55;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0x55, cpu_state.registers.V[0]);
}

/**
 * Test 0xFX0A: KEY Vx
 * Wait for key press and stores the value in V[X]
 */
void test_op_FX0A_key_pressed(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF00A, 0);
  cpu_state.peripherals.keypad[5] = 1;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(5, cpu_state.registers.V[0]);
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS + 2, cpu_state.registers.pc);
}

/**
 * Test 0xFX0A: KEY Vx
 * Wait for key press and stores the value in V[X]
 */
void test_op_FX0A_key_not_pressed(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF00A, 0);

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(START_ADDRESS, cpu_state.registers.pc);
}

/**
 * Test 0xFX15: SDLY Vx
 * Set the delay timer to the value stored in V[X]
 */
void test_op_FX15(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF015, 0);
  cpu_state.registers.V[0] = 0x55;
  cpu_state.timers.delay = 0xAA;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0x54, cpu_state.timers.delay);
}

/**
 * Test 0xFX18: SSND Vx
 * Set the sound timer to the value stored in V[X]
 */
void test_op_FX18(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF018, 0);
  cpu_state.registers.V[0] = 0x55;
  cpu_state.timers.sound = 0xAA;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX8(0x54, cpu_state.timers.sound);
}

/**
 * Test 0xFX1E: ADI Vx
 * Add register V[X] to the index register
 */
void test_op_FX1E(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF01E, 0);
  cpu_state.registers.V[0] = 0x23;
  cpu_state.registers.I = 0x5532;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16(0x5555, cpu_state.registers.I);
}

/**
 * Test 0xFX29: FONT Vx
 * Sets I to the location of the sprite for the character in V[X]
 */
void test_op_FX29(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF029, 0);
  cpu_state.registers.V[0] = 0x55;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_HEX16((0x55 * 5), cpu_state.registers.I);
}

/**
 * Test 0xFX33: BCD Vx
 * Stores the binary-coded decimal representation of V[X],
 * with the hundreds digit in memory at location in I,
 * the tens digit at location I+1,
 * and the ones digit at location I+2.
 * Note: This doesn't change I
 */
void test_op_FX33_with_valid_address(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF033, 0);
  cpu_state.registers.V[0] = 123;
  cpu_state.registers.I = 0x0FF0;

  TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
  TEST_ASSERT_EQUAL_INT8(1, cpu_state.memory[0x0FF0]);
  TEST_ASSERT_EQUAL_INT8(2, cpu_state.memory[0x0FF1]);
  TEST_ASSERT_EQUAL_INT8(3, cpu_state.memory[0x0FF2]);
  TEST_ASSERT_EQUAL_HEX16(0x0FF0, cpu_state.registers.I);
}

/**
 * Test 0xFX33: BCD Vx
 * Stores the binary-coded decimal representation of V[X],
 * with the hundreds digit in memory at location in I,
 * the tens digit at location I+1,
 * and the ones digit at location I+2.
 * Note: This doesn't change I
 */
void test_op_FX33_with_out_of_bound_address(void)
{
  cpu_state_t cpu_state = {0};
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF033, 0);
  cpu_state.registers.V[0] = 123;
  cpu_state.registers.I = 0x0FFE;

  TEST_ASSERT_EQUAL_INT(STATUS_ERR_MEM_OUT_OF_BOUNDS, emulation_cycle(&cpu_state));
}

/**
 * Test 0xFX55: STR V0, Vx
 * Stores the values in V[0] to V[X] (including V[X]) in memory, starting at address I.
 * Note: This doesn't change I
 */
void test_op_FX55_with_valid_addresses(void)
{
  cpu_state_t cpu_state = {0};

  cpu_state.registers.I = 0x0FF0;
  for (int8_t i = 0; i < REG_COUNT; i++)
  {
    cpu_state.registers.V[i] = (0xA0 | i);
  }

  for (int8_t i = 0; i < REG_COUNT; i++)
  {
    stub_clear_mem(&cpu_state);
    stub_init_cpu_state(&cpu_state);
    stub_set_opcode(&cpu_state, (0xF055 | (i << 8)), 0);

    TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
    for (int8_t j = 0; j <= REG_COUNT; j++)
    {
      uint16_t offset = cpu_state.registers.I + j;
      uint8_t expected = (j > i) ? 0x00 : cpu_state.registers.V[j];
      TEST_ASSERT_EQUAL_HEX8(expected, cpu_state.memory[offset]);
    }
    TEST_ASSERT_EQUAL_HEX16(0x0FF0, cpu_state.registers.I);
  }
}
/**
 * Test 0xFX55: STR V0, Vx
 * Stores the values in V[0] to V[X] (including V[X]) in memory, starting at address I.
 * Note: This doesn't change I
 */
void test_op_FX55_with_out_of_bound_address(void)
{
  cpu_state_t cpu_state = {0};

  stub_clear_mem(&cpu_state);
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF155, 0);
  cpu_state.registers.I = 0x0FFF;

  TEST_ASSERT_EQUAL_INT(STATUS_ERR_MEM_OUT_OF_BOUNDS, emulation_cycle(&cpu_state));
}

/**
 * Test 0xFX65: LDR V0, Vx
 * Fills from V[0] to V[X] (including V[X]) with values from memory, starting at address I.
 * Note: This doesn't change I
 */
void test_op_FX65_with_valid_addresses(void)
{
  cpu_state_t cpu_state = {0};
  cpu_state.registers.I = 0x0FF0;

  for (int8_t i = 0; i < REG_COUNT; i++)
  {
    cpu_state.memory[cpu_state.registers.I + i] = 0xA0 | i;
  }

  for (int8_t i = 0; i < REG_COUNT; i++)
  {
    stub_clear_V(&cpu_state);
    stub_init_cpu_state(&cpu_state);
    stub_set_opcode(&cpu_state, (0xF065 | (i << 8)), 0);

    TEST_ASSERT_EQUAL_INT(STATUS_OK, emulation_cycle(&cpu_state));
    for (int8_t j = 0; j <= REG_COUNT; j++)
    {
      uint8_t expected = (j > i) ? 0x00 : cpu_state.memory[cpu_state.registers.I + j];
      TEST_ASSERT_EQUAL_HEX8(expected, cpu_state.registers.V[j]);
    }
    TEST_ASSERT_EQUAL_HEX16(0x0FF0, cpu_state.registers.I);
  }
}

/**
 * Test 0xFX65: LDR V0, Vx
 * Fills from V[0] to V[X] (including V[X]) with values from memory, starting at address I.
 * Note: This doesn't change I
 */
void test_op_FX65_with_out_of_bound_address(void)
{
  cpu_state_t cpu_state = {0};

  stub_clear_V(&cpu_state);
  stub_init_cpu_state(&cpu_state);
  stub_set_opcode(&cpu_state, 0xF165, 0);
  cpu_state.registers.I = 0x0FFF;

  TEST_ASSERT_EQUAL_INT(STATUS_ERR_MEM_OUT_OF_BOUNDS, emulation_cycle(&cpu_state));
}