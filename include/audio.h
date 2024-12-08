#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "status_code.h"

status_code_t audio_init();
status_code_t audio_play_beep();
status_code_t audio_cleanup();

#endif /* __AUDIO_H__ */
