#include <stdint.h>
#include <SDL2/SDL.h>

#include "cpu_def.h"
#include "keypad.h"
#include "status_code.h"

static const SDL_Scancode key_map[16] = {
    SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
    SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
    SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V};

status_code_t keypad_read(uint8_t *const key_buf)
{
  VERIFY_PTR_RETURN_ERROR_IF_NULL(key_buf);

  SDL_Event event;
  status_code_t status = STATUS_OK;

  if (!SDL_PollEvent(&event))
  {
    return status;
  }

  const uint8_t *key_state = SDL_GetKeyboardState(NULL);

  if (event.type == SDL_QUIT)
  {
    return STATUS_REQ_EXIT;
  }

  if (key_state[SDL_SCANCODE_ESCAPE])
  {
    status = STATUS_REQ_EXIT;
  }

  for (int8_t i = 0; i < NUM_KEYS; i++)
  {
    key_buf[i] = key_state[key_map[i]];
  }

  return status;
}
