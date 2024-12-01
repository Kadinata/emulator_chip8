#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdint.h>

#include "status_code.h"
#include "cpu_def.h"

/**
 * Initializes the display and allocate resources for it.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t display_init();

/**
 * Render the contents of the provided graphics buffer onto the display
 * The display is updated only when the display_update flag is set.
 * @param graphics - Pointer to the graphics buffer whose contents are to be
 *                   rendered on the display.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t display_render(graphics_t * const graphics);

/**
 * Cleanup and free display resources
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t display_cleanup();

#endif /* __DISPLAY_H__ */
