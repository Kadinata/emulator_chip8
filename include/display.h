#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <stdint.h>

#include "status_code.h"
#include "cpu_def.h"

#define DEFAULT_FG_COLOR ((color_rgba_t){.r = 0xE9, .g = 0xE9, .b = 0xE9, .a = 0xFF})
#define DEFAULT_BG_COLOR ((color_rgba_t){.r = 0x12, .g = 0x12, .b = 0x12, .a = 0xFF})

/** Structure to encode color in RGBA format */
typedef struct __attribute__((packed)) color_rgba_s
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} color_rgba_t;

/** Parameters to initialize the display module */
typedef struct display_init_param_s
{
  /** Desired color for the foreground "ON" pixels */
  color_rgba_t foreground_color;

  /** Desired color for the background "OFF" pixels */
  color_rgba_t background_color;
} display_init_param_t;

/**
 * Initializes the display and allocate resources for it.
 * @param title - The desired title of the window to be created.
 * @param param - Pointer to an initialization parameters struct.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t display_init(const char *title, display_init_param_t *const param);

/**
 * Render the contents of the provided graphics buffer onto the display
 * The display is updated only when the display_update flag is set.
 * @param graphics - Pointer to the graphics buffer whose contents are to be
 *                   rendered on the display.
 * @return STATUS_OK if successful, otherwise appropriate error code.
 */
status_code_t display_render(graphics_t *const graphics);

/**
 * Cleanup and free display resources
 * @return None
 */
void display_cleanup();

#endif /* __DISPLAY_H__ */
