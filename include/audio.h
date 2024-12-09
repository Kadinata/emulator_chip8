#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <stdint.h>
#include "status_code.h"

#define DEFAULT_SAMPLE_FREQ_HZ (44100) // 44.1 kHz audio sampling rate
#define DEFAULT_TONE_FREQ_HZ (261)     // Default beep note, roughly the frequency of the middle C in the A440 pitch
#define DEFAULT_VOLUME (3000)          // Audio volume level from 0 to 32767

/** Parameters to initialize the audio module */
typedef struct audio_init_param_s
{
  /** Desired audio sampling rate in Hz, default is 44.1 kHz */
  uint32_t sample_freq_hz;

  /** Desired beep tone frequency, default is 261 Hz (middle C) */
  uint32_t tone_freq_hz;
} audio_init_param_t;

/**
 * Initializes the audio module and allocate resources for it
 * @param param - Pointer to an initialization parameters struct.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t audio_init(audio_init_param_t *const param);

/**
 * Emit tone with a frequency that's configured during initialization.
 * The tone will continue to be emitted until audio_mute is called.
 * This function should be called when the sound timer is greater than 0.
 * @return None
 */
void audio_play_beep();

/**
 * Stop the tone. This should be called when the sound timer is 0.
 * @return None
 */
void audio_mute();

/**
 * Cleanup and free audio resources
 * @return None
 */
void audio_cleanup();

#endif /* __AUDIO_H__ */
