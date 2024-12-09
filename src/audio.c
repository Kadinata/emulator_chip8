#include <stdint.h>
#include <SDL2/SDL.h>

#include "audio.h"
#include "logging.h"
#include "status_code.h"

typedef struct audio_handle_s {
  SDL_AudioDeviceID audio_device;
  uint32_t sample_freq_hz;
  uint32_t tone_freq_hz;
} audio_handle_t;

static audio_handle_t audio_handle;

static void audio_callback(void  __attribute__((unused)) *userdata, uint8_t * audio_buffer, int len)
{
  static uint32_t sample_num = 0;

  for (int i = 0 ; i < len; i++)
  {
    audio_buffer[i] = (sample_num / ((audio_handle.sample_freq_hz / audio_handle.tone_freq_hz) / 2)) & 0x1 ? -DEFAULT_VOLUME : DEFAULT_VOLUME;
    sample_num++;
  }
}

status_code_t audio_init(audio_init_param_t *const param)
{
  VERIFY_PTR_RETURN_ERROR_IF_NULL(param);

  if ((param->sample_freq_hz == 0) || (param->tone_freq_hz == 0))
  {
    return STATUS_ERR_MATH_DIV_0;
  }

  int16_t init_result;
  if ((init_result = SDL_InitSubSystem(SDL_INIT_AUDIO)) != 0) {
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

status_code_t audio_play_beep()
{
  SDL_PauseAudioDevice(audio_handle.audio_device, 0);
  return STATUS_OK;
}

status_code_t audio_mute()
{
  SDL_PauseAudioDevice(audio_handle.audio_device, 1);
  return STATUS_OK;
}

status_code_t audio_cleanup()
{
  SDL_CloseAudioDevice(audio_handle.audio_device);
  return STATUS_OK;
}
