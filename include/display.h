#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdint.h>

#include "status_code.h"
#include "cpu_def.h"

status_code_t display_init();
status_code_t display_render(graphics_t * const graphics);
status_code_t display_cleanup();

#endif
