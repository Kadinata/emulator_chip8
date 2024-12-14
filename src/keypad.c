#include <stdint.h>
#include <SDL2/SDL.h>

#include "cpu_def.h"
#include "keypad.h"
#include "status_code.h"

static const SDL_Scancode key_map[NUM_KEYS] = {
    SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
    SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
    SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V};

status_code_t keypad_read(keypad_state_t *const keypad)
{
  VERIFY_PTR_RETURN_ERROR_IF_NULL(keypad);

  SDL_Event event;
  status_code_t status = STATUS_OK;

  if (!SDL_PollEvent(&event))
  {
    return status;
  }

  const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);

  if (event.type == SDL_QUIT)
  {
    return STATUS_REQ_EXIT;
  }

  if (keyboard_state[SDL_SCANCODE_ESCAPE])
  {
    status = STATUS_REQ_EXIT;
  }

  keypad->previous = keypad->current;
  keypad->current = 0;

  for (int8_t i = 0; i < NUM_KEYS; i++)
  {
    keypad->current |= keyboard_state[key_map[i]] ? (1 << i) : 0;
  }

  return status;
}
