#include <stdint.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "display.h"
#include "cpu_def.h"
#include "logging.h"
#include "status_code.h"

#define PIXEL_WIDTH (8)

typedef struct display_handle_s
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  color_rgba_t fg_color;
  color_rgba_t bg_color;
} display_handle_t;

static display_handle_t display_handle;

status_code_t display_init(const char *title, display_init_param_t *const param)
{
  Log_I("Initializing the display module...");

  VERIFY_PTR_RETURN_ERROR_IF_NULL(title);
  VERIFY_PTR_RETURN_ERROR_IF_NULL(param);

  int16_t init_result;
  if ((init_result = SDL_InitSubSystem(SDL_INIT_VIDEO)) != 0)
  {
    Log_E("Failed to inittialize SDL Video Subsystem (%d)", init_result);
    return STATUS_ERR_GENERIC;
  }

  display_handle.window = SDL_CreateWindow(
      title,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      (GRAPHICS_WIDTH * PIXEL_WIDTH),
      (GRAPHICS_HEIGHT * PIXEL_WIDTH),
      0);

  display_handle.renderer = SDL_CreateRenderer(display_handle.window, -1, SDL_RENDERER_ACCELERATED);

  memcpy(&display_handle.bg_color, &param->background_color, sizeof(color_rgba_t));
  memcpy(&display_handle.fg_color, &param->foreground_color, sizeof(color_rgba_t));

  Log_I("Display module successfully initialized.");
  return STATUS_OK;
}

status_code_t display_render(graphics_t *const graphics)
{

  VERIFY_PTR_RETURN_ERROR_IF_NULL(graphics);
  if (!graphics->display_update)
  {
    return STATUS_OK;
  }

  graphics->display_update = 0;
  color_rgba_t const *fg_color = &display_handle.fg_color;
  color_rgba_t const *bg_color = &display_handle.bg_color;

  SDL_SetRenderDrawColor(display_handle.renderer, bg_color->r, bg_color->g, bg_color->b, bg_color->a);
  SDL_RenderClear(display_handle.renderer);

  SDL_SetRenderDrawColor(display_handle.renderer, fg_color->r, fg_color->g, fg_color->b, fg_color->a);

  for (uint8_t row = 0; row < GRAPHICS_HEIGHT; row++)
  {
    for (uint8_t col = 0; col < GRAPHICS_WIDTH; col++)
    {
      if (graphics->buffer[col + (row * GRAPHICS_WIDTH)])
      {
        SDL_Rect rect;

        rect.x = col * PIXEL_WIDTH;
        rect.y = row * PIXEL_WIDTH;
        rect.w = PIXEL_WIDTH;
        rect.h = PIXEL_WIDTH;

        SDL_RenderFillRect(display_handle.renderer, &rect);
      }
    }
  }

  SDL_RenderPresent(display_handle.renderer);
  return STATUS_OK;
}

void display_cleanup()
{
  Log_I("Cleaning up the display module.");
  SDL_DestroyRenderer(display_handle.renderer);
  SDL_DestroyWindow(display_handle.window);
  SDL_Quit();
}
