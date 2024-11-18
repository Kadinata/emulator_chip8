#include <stdint.h>
#include <SDL2/SDL.h>

#include "display.h"
#include "cpu_def.h"
#include "status_code.h"

#define PIXEL_WIDTH (8)

typedef struct display_handle_s
{
  SDL_Window *window;
  SDL_Renderer *renderer;
} display_handle_t;

static display_handle_t display_handle;

status_code_t display_init(const uint8_t *title)
{
  VERIFY_PTR_RETURN_ERROR_IF_NULL(title);

  SDL_Init(SDL_INIT_VIDEO);
  display_handle.window = SDL_CreateWindow(
      (char *)title,
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      GRAPHICS_WIDTH * 8,
      GRAPHICS_HEIGHT * 8,
      0);

  display_handle.renderer = SDL_CreateRenderer(display_handle.window, -1, SDL_RENDERER_ACCELERATED);

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

  SDL_SetRenderDrawColor(display_handle.renderer, 0x12, 0x12, 0x12, 0xFF);
  SDL_RenderClear(display_handle.renderer);

  SDL_SetRenderDrawColor(display_handle.renderer, 0xE9, 0xE9, 0xE9, 0xFF);

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

status_code_t display_cleanup()
{
  SDL_DestroyRenderer(display_handle.renderer);
  SDL_DestroyWindow(display_handle.window);
  SDL_Quit();

  return STATUS_OK;
}
