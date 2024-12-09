#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "audio.h"
#include "logging.h"
#include "status_code.h"

/**
 * Data structure definition to keep track
 * of the audio module's internal state
 */
typedef struct audio_handle_s
{
  SDL_AudioDeviceID audio_device;
  uint32_t sample_freq_hz;
  uint32_t tone_freq_hz;
} audio_handle_t;

static audio_handle_t audio_handle;

/**
 * Handler function to populate SDL's audio output buffer with audio samples. In this case,
 * the buffer will be populated with triangular wave samples.
 * @param userdata - Pointer to custom user data (unused)
 * @param audio_buffer - Audio output buffer provided by SDL
 * @param len - Number of samples requested by SDL
 * @return - None
 */
static void audio_callback(void __attribute__((unused)) * userdata, uint8_t *audio_buffer, int len)
{
  static uint32_t sample_num = 0;

  int32_t output;
  int32_t samples_per_period = audio_handle.sample_freq_hz / audio_handle.tone_freq_hz;

  /**
   * Generate samples of a triangular wave and fill the audio output buffer with them.
   * One period of the triangular wave is based on the scaled and shifted version of
   * y = abs(x) where the range of x is [-1, 1], where x corresponds to the sample index
   * or sample number. With scaling and shifts applied, the equation becomes:
   *    y = abs(slope * (x - x_offset)) + y_offset
   *
   * Finding the slope
   *    slope = rise / run
   *    slope = amplitude / (sps / 2)
   *    slope = 2 * amplitude / sps, where sps = number of samples per wave period
   *
   * The amplitude ranges from -volume to +volume; therefore:
   *    amplitude = 2 * volume
   *    slope = 4 * volume / sps
   *
   * x_offset should shift the wave a half period to the right; therefore:
   *    x_offset = sps / 2
   *
   * y_offset should center the wave around 0; therefore:
   *    y_offset = -amplitude / 2
   *    y_offset = -volume
   *
   * Putting it together:
   *    y = abs((4 * volume / sps) * (x - sps / 2)) - volume
   *    y = abs((4 * volume * x / sps) - (4 * volume * sps) * (sps / 2)) - volume
   *    y = abs((4 * volume * x / sps) - (2 * volume)) - volume
   */
  for (int i = 0; i < len; i++)
  {
    output = 4 * DEFAULT_VOLUME * sample_num / samples_per_period;
    output = abs(output - (2 * DEFAULT_VOLUME)) - DEFAULT_VOLUME;
    audio_buffer[i] = (int8_t)output;
    sample_num++;
    sample_num %= samples_per_period;
  }
}

status_code_t audio_init(audio_init_param_t *const param)
{
  Log_I("Initializing the audio module...");

  VERIFY_PTR_RETURN_ERROR_IF_NULL(param);

  if ((param->sample_freq_hz == 0) || (param->tone_freq_hz == 0))
  {
    return STATUS_ERR_MATH_DIV_0;
  }

  int16_t init_result;
  if ((init_result = SDL_InitSubSystem(SDL_INIT_AUDIO)) != 0)
  {
    Log_E("Failed to inittialize SDL Audio Subsystem (%d)", init_result);
    return STATUS_ERR_GENERIC;
  }

  SDL_AudioSpec desired_spec = (SDL_AudioSpec){
      .freq = param->sample_freq_hz,
      .format = AUDIO_S8,
      .channels = 1,
      .samples = 512,
      .callback = audio_callback,
      .userdata = NULL,
  };

  SDL_AudioSpec obtained_spec;

  audio_handle.audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &obtained_spec, 0);
  audio_handle.sample_freq_hz = param->sample_freq_hz;
  audio_handle.tone_freq_hz = param->tone_freq_hz;

  if (audio_handle.audio_device == 0)
  {
    Log_E("Failed to open audio device.");
    return STATUS_ERR_GENERIC;
  }

  status_code_t status = STATUS_OK;

  if (desired_spec.freq != obtained_spec.freq)
  {
    Log_E("Failed to obtain the desired audio sample rate. Desired: %d Hz; obtained: %d Hz", desired_spec.freq, obtained_spec.freq);
    status = STATUS_ERR_GENERIC;
  }

  if (desired_spec.format != obtained_spec.format)
  {
    Log_E("Failed to obtain the desired audio format. Desired: 0x%04X; obtained: 0x%04X", desired_spec.format, obtained_spec.format);
    status = STATUS_ERR_GENERIC;
  }

  if (desired_spec.channels != obtained_spec.channels)
  {
    Log_E("Failed to obtain the desired number of audio channels. Desired: %d; obtained: %d", desired_spec.channels, obtained_spec.channels);
    status = STATUS_ERR_GENERIC;
  }

  if (desired_spec.samples != obtained_spec.samples)
  {
    Log_E("Failed to obtain the desired number of audio samples. Desired: %d; obtained: %d", desired_spec.samples, obtained_spec.samples);
    status = STATUS_ERR_GENERIC;
  }

  if (status == STATUS_OK)
  {
    Log_I("Audio module successfully initialized.");
  }

  return status;
}

void audio_play_beep()
{
  SDL_PauseAudioDevice(audio_handle.audio_device, 0);
}

void audio_mute()
{
  SDL_PauseAudioDevice(audio_handle.audio_device, 1);
}

void audio_cleanup()
{
  Log_I("Cleaning up the audio module.");
  SDL_CloseAudioDevice(audio_handle.audio_device);
}
