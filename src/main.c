#include "game.h"
#include "platform.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define TARGET_FPS  60
#define FRAME_MS    (1000 / TARGET_FPS)

static GameState gs;
static int running = 1;

static void main_loop(void)
{
    int left, right, fire, quit;
#ifndef __EMSCRIPTEN__
    uint32_t t0, elapsed;
    t0 = plat_ticks();
#endif

    quit = plat_poll_input(&left, &right, &fire);
    if (quit) {
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        running = 0;
        return;
    }

    if ((gs.game_over || gs.won) && fire) {
        game_init(&gs);
    }

    game_update(&gs, left, right, fire);
    game_draw(&gs);

#ifndef __EMSCRIPTEN__
    elapsed = plat_ticks() - t0;
    if (elapsed < FRAME_MS)
        plat_delay(FRAME_MS - elapsed);
#endif
}

int main(void)
{
    if (plat_init(SCREEN_W, SCREEN_H, "Breakout") != 0)
        return 1;

    game_init(&gs);

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    for (;;) {
        main_loop();
        if (!running)
            break;
    }
#endif

    plat_shutdown();
    return 0;
}
