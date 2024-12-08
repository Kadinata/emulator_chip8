#include <stdint.h>
#include <SDL2/SDL.h>

#include "audio.h"
#include "logging.h"
#include "status_code.h"

status_code_t audio_init()
{
  int16_t init_result;
  if ((init_result = SDL_InitSubSystem(SDL_INIT_AUDIO)) != 0) {
    Log_E("Failed to inittialize SDL Audio Subsystem (%d)", init_result);
    return STATUS_ERR_GENERIC;
  }

  return STATUS_OK;
}

status_code_t audio_play_beep()
{
  return STATUS_OK;
}

status_code_t audio_cleanup()
{
  return STATUS_OK;
}
