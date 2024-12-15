#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <SDL2/SDL.h>

#include "chip8.h"
#include "keypad.h"
#include "audio.h"
#include "display.h"
#include "logging.h"
#include "timer.h"

#define WINDOW_TITLE ("Chip-8 Emulator")
#define LOOP_FREQ_HZ (700)
#define DISPLAY_FREQ_HZ (60)

void print_usage(void)
{
  printf("\nUsage: chip8_emu.out <ROM file>\n");
}

void cleanup()
{
  audio_cleanup();
  display_cleanup();
}

int main(int argc, char **argv)
{

  cpu_state_t cpu_state = {0};
  timer_t system_timer, display_timer;
  status_code_t status = STATUS_OK;
  uint8_t main_loop = 1;
  audio_init_param_t audio_init_param = (audio_init_param_t){
      .sample_freq_hz = DEFAULT_SAMPLE_FREQ_HZ,
      .tone_freq_hz = DEFAULT_TONE_FREQ_HZ,
  };
  display_init_param_t display_init_param = (display_init_param_t){
      .background_color = DEFAULT_BG_COLOR,
      .foreground_color = DEFAULT_FG_COLOR,
  };

  if (argc != 2)
  {
    print_usage();
    return STATUS_ERR_GENERIC;
  }

  // Initialize the CPU
  Log_I("Initializing CPU...");
  status = init_cpu(&cpu_state);
  if (status != STATUS_OK)
  {
    Log_E("An error occurred while initializing CPU: %u", status);
    return status;
  }
  Log_I("CPU Init complete.");

  // Load ROM file content to memory
  Log_I("Loading ROM file: %s", argv[1]);
  status = load_rom(&cpu_state, argv[1]);
  if (status != STATUS_OK)
  {
    Log_E("An error occurred while loading ROM: %u", status);
    return status;
  }
  Log_I("ROM loaded succesfully.");

  // Initialize system frequency timer
  Log_I("Initializing system timer...");
  status = timer_init(&system_timer, LOOP_FREQ_HZ);
  if (status != STATUS_OK)
  {
    Log_E("An error occurred while initializing system timer: %u", status);
    return status;
  }
  Log_I("System timer initialized successfully.");

  // Initialize display and audio timer
  Log_I("Initializing 60 Hz display timer...");
  status = timer_init(&display_timer, DISPLAY_FREQ_HZ);
  if (status != STATUS_OK)
  {
    Log_E("An error occurred while initializing the 60 Hz display timer: %u", status);
    return status;
  }
  Log_I("60 Hz display timer initialized successfully.");

  // Initialize the display module
  status = display_init(WINDOW_TITLE, &display_init_param);
  if (status != STATUS_OK)
  {
    cleanup();
    return status;
  }

  // Initialize the audio module
  status = audio_init(&audio_init_param);
  if (status != STATUS_OK)
  {
    cleanup();
    return status;
  }

  Log_I("Starting the main execution loop");
  while (main_loop)
  {

    status = keypad_read(&cpu_state.peripherals.keypad.current);
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

    if (timer_check(&system_timer))
    {
      status = emulation_cycle(&cpu_state);
      if (status != STATUS_OK)
      {
        Log_F("Emulation cycle encountered an error: %u", status);
        main_loop = 0;
      }
    }

    if (timer_check(&display_timer))
    {
      ((cpu_state.timers.sound > 0) ? audio_play_beep() : audio_mute());
      update_timers(&cpu_state);

      status = display_render(&cpu_state.peripherals.graphics);
      if (status != STATUS_OK)
      {
        Log_F("Display rendering encountered an error: %u", status);
        main_loop = 0;
      }
    }
  }

  cleanup();
  return status;
}
