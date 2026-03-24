#include "game.h"
#include "platform.h"

#define TARGET_FPS  60
#define FRAME_MS    (1000 / TARGET_FPS)

int main(void)
{
    GameState gs;
    uint32_t t0, elapsed;
    int left, right, fire, quit;

    if (plat_init(SCREEN_W, SCREEN_H, "Breakout") != 0)
        return 1;

    game_init(&gs);

    for (;;) {
        t0 = plat_ticks();

        quit = plat_poll_input(&left, &right, &fire);
        if (quit)
            break;

        if ((gs.game_over || gs.won) && fire) {
            game_init(&gs);
        }

        game_update(&gs, left, right, fire);
        game_draw(&gs);

        elapsed = plat_ticks() - t0;
        if (elapsed < FRAME_MS)
            plat_delay(FRAME_MS - elapsed);
    }

    plat_shutdown();
    return 0;
}
