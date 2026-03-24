#include "platform.h"
#include <SDL.h>

static SDL_Window   *win;
static SDL_Renderer *ren;
static int           scale = 2;  /* 320x256 → 640x512 on macOS */

int plat_init(int w, int h, const char *title)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return -1;

    win = SDL_CreateWindow(title,
                           SDL_WINDOWPOS_CENTERED,
                           SDL_WINDOWPOS_CENTERED,
                           w * scale, h * scale,
                           0);
    if (!win)
        return -1;

    ren = SDL_CreateRenderer(win, -1,
                             SDL_RENDERER_ACCELERATED |
                             SDL_RENDERER_PRESENTVSYNC);
    if (!ren)
        return -1;

    SDL_RenderSetLogicalSize(ren, w, h);
    return 0;
}

void plat_shutdown(void)
{
    if (ren) SDL_DestroyRenderer(ren);
    if (win) SDL_DestroyWindow(win);
    SDL_Quit();
}

void plat_clear(uint8_t r, uint8_t g, uint8_t b)
{
    SDL_SetRenderDrawColor(ren, r, g, b, 255);
    SDL_RenderClear(ren);
}

void plat_draw_rect(int x, int y, int w, int h,
                    uint8_t r, uint8_t g, uint8_t b)
{
    SDL_Rect rc = { x, y, w, h };
    SDL_SetRenderDrawColor(ren, r, g, b, 255);
    SDL_RenderFillRect(ren, &rc);
}

void plat_flip(void)
{
    SDL_RenderPresent(ren);
}

int plat_poll_input(int *left, int *right, int *fire)
{
    SDL_Event e;
    const uint8_t *keys;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT)
            return 1;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
            return 1;
    }

    keys = SDL_GetKeyboardState(NULL);
    *left  = keys[SDL_SCANCODE_LEFT]  || keys[SDL_SCANCODE_A];
    *right = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];
    *fire  = keys[SDL_SCANCODE_SPACE];

    return 0;
}

void plat_delay(int ms)
{
    SDL_Delay((uint32_t)ms);
}

uint32_t plat_ticks(void)
{
    return SDL_GetTicks();
}
