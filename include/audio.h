#ifndef __AUDIO_H__
#define __AUDIO_H__

#include <stdint.h>
#include "status_code.h"

#define DEFAULT_SAMPLE_FREQ_HZ (44100) // 44.1 kHz audio sampling rate
#define DEFAULT_TONE_FREQ_HZ (261)     // Default beep note, roughly the frequency of the middle C
#define DEFAULT_VOLUME (12)            // Audio volume level from 0 to 127

typedef struct audio_init_param_s
{
  uint32_t sample_freq_hz;
  uint32_t tone_freq_hz;
} audio_init_param_t;

status_code_t audio_init(audio_init_param_t *const param);
status_code_t audio_play_beep();
status_code_t audio_mute();
status_code_t audio_cleanup();

#endif /* __AUDIO_H__ */
