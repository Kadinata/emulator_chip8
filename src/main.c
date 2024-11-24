#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <SDL2/SDL.h>

#include "chip8.h"
#include "keypad.h"
#include "display.h"
#include "logging.h"

#define WINDOW_TITLE ("Chip-8 Emulator")

void print_usage(void)
{
  printf("\nUsage: chip8_emu.out <ROM file>\n");
}

int main(int argc, char **argv)
{

  cpu_state_t cpu_state = {0};
  status_code_t status = STATUS_OK;
  uint8_t main_loop = 1;

  if (argc != 2)
  {
    print_usage();
    return STATUS_ERR_GENERIC;
  }

  Log_I("Initializing CPU...");
  status = init_cpu(&cpu_state);
  if (status != STATUS_OK)
  {
    Log_E("An error occurred while initializing CPU: %u", status);
    return status;
  }
  Log_I("CPU Init complete.");

  Log_I("Loading ROM file: %s", argv[1]);
  status = load_rom(&cpu_state, argv[1]);
  if (status != STATUS_OK)
  {
    Log_E("An error occurred while loading ROM: %u", status);
    return status;
  }
  Log_I("ROM loaded succesfully.");

  Log_I("Setting up display...");
  status = display_init(WINDOW_TITLE);
  if (status != STATUS_OK)
  {
    Log_E("An error occurred while initializing display: %u", status);
    return display_cleanup();
  }

  Log_I("Display initialized successfully.");

  while (main_loop)
  {

    status = keypad_read(cpu_state.peripherals.keypad);
    if (status == STATUS_REQ_EXIT)
    {
      Log_I("Exiting...");
      main_loop = 0;
    }
    else if (status != STATUS_OK)
    {
      Log_F("Keypad reading encountered an error: %u", status);
      main_loop = 0;
    }

    status = emulation_cycle(&cpu_state);
    if (status != STATUS_OK)
    {
      Log_F("Emulation cycle encountered an error: %u", status);
      main_loop = 0;
    }

    status = display_render(&cpu_state.peripherals.graphics);
    if (status != STATUS_OK)
    {
      Log_F("Display rendering encountered an error: %u", status);
      main_loop = 0;
    }

    /** 2 msec delay == 500 Hz cycle */
    SDL_Delay(2);
  }

  status = display_cleanup();
  if (status != STATUS_OK)
  {
    Log_E("An error occurred while cleaning up display: %u", status);
  }
  return status;
}
