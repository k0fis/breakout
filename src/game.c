#include "game.h"
#include "platform.h"

/* Amiga-friendly palette: bold, saturated colors */
static const uint8_t brick_colors[BRICK_ROWS][3] = {
    {0xFF, 0x22, 0x22},  /* red */
    {0xFF, 0x88, 0x00},  /* orange */
    {0xFF, 0xDD, 0x00},  /* yellow */
    {0x00, 0xCC, 0x44},  /* green */
    {0x22, 0x88, 0xFF},  /* blue */
};

void game_init(GameState *gs)
{
    int r, c;

    gs->paddle_x = (SCREEN_W - PADDLE_W) / 2;

    gs->ball_dx = BALL_SPEED;
    gs->ball_dy = -BALL_SPEED;
    gs->ball_active = 0;

    gs->score = 0;
    gs->lives = MAX_LIVES;
    gs->game_over = 0;
    gs->won = 0;
    gs->bricks_alive = 0;

    for (r = 0; r < BRICK_ROWS; r++) {
        for (c = 0; c < BRICK_COLS; c++) {
            Brick *b = &gs->bricks[r][c];
            b->x = BRICKS_X0 + c * (BRICK_W + BRICK_GAP);
            b->y = BRICKS_Y0 + r * (BRICK_H + BRICK_GAP);
            b->alive = 1;
            b->r = brick_colors[r][0];
            b->g = brick_colors[r][1];
            b->b = brick_colors[r][2];
            gs->bricks_alive++;
        }
    }
}

static void reset_ball(GameState *gs)
{
    gs->ball_active = 0;
    gs->ball_dx = BALL_SPEED;
    gs->ball_dy = -BALL_SPEED;
}

static int rects_overlap(int ax, int ay, int aw, int ah,
                         int bx, int by, int bw, int bh)
{
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

void game_update(GameState *gs, int left, int right, int fire)
{
    int nx, ny;
    int r, c;

    if (gs->game_over || gs->won)
        return;

    /* paddle movement */
    if (left)
        gs->paddle_x -= PADDLE_SPEED;
    if (right)
        gs->paddle_x += PADDLE_SPEED;

    if (gs->paddle_x < 0)
        gs->paddle_x = 0;
    if (gs->paddle_x > SCREEN_W - PADDLE_W)
        gs->paddle_x = SCREEN_W - PADDLE_W;

    /* ball stuck to paddle */
    if (!gs->ball_active) {
        gs->ball_x = gs->paddle_x + PADDLE_W / 2 - BALL_SIZE / 2;
        gs->ball_y = PADDLE_Y - BALL_SIZE;
        if (fire) {
            gs->ball_active = 1;
        }
        return;
    }

    /* move ball */
    nx = gs->ball_x + gs->ball_dx;
    ny = gs->ball_y + gs->ball_dy;

    /* wall collisions */
    if (nx < 0) {
        nx = 0;
        gs->ball_dx = -gs->ball_dx;
    }
    if (nx > SCREEN_W - BALL_SIZE) {
        nx = SCREEN_W - BALL_SIZE;
        gs->ball_dx = -gs->ball_dx;
    }
    if (ny < 0) {
        ny = 0;
        gs->ball_dy = -gs->ball_dy;
    }

    /* ball fell off bottom */
    if (ny > SCREEN_H) {
        gs->lives--;
        if (gs->lives <= 0) {
            gs->game_over = 1;
        } else {
            reset_ball(gs);
        }
        return;
    }

    /* paddle collision */
    if (rects_overlap(nx, ny, BALL_SIZE, BALL_SIZE,
                      gs->paddle_x, PADDLE_Y, PADDLE_W, PADDLE_H)) {
        /* reflect upward */
        ny = PADDLE_Y - BALL_SIZE;
        gs->ball_dy = -gs->ball_dy;

        /* angle based on where ball hit paddle */
        {
            int center = gs->paddle_x + PADDLE_W / 2;
            int diff = (nx + BALL_SIZE / 2) - center;
            /* shift dx by -2..+2 based on hit position */
            gs->ball_dx = diff / 6;
            if (gs->ball_dx == 0)
                gs->ball_dx = (gs->ball_dx >= 0) ? 1 : -1;
            /* clamp speed */
            if (gs->ball_dx > BALL_SPEED)
                gs->ball_dx = BALL_SPEED;
            if (gs->ball_dx < -BALL_SPEED)
                gs->ball_dx = -BALL_SPEED;
        }
    }

    /* brick collisions */
    for (r = 0; r < BRICK_ROWS; r++) {
        for (c = 0; c < BRICK_COLS; c++) {
            Brick *b = &gs->bricks[r][c];
            if (!b->alive)
                continue;
            if (rects_overlap(nx, ny, BALL_SIZE, BALL_SIZE,
                              b->x, b->y, BRICK_W, BRICK_H)) {
                b->alive = 0;
                gs->bricks_alive--;
                gs->score += 10 * (BRICK_ROWS - r);

                /* determine bounce direction */
                {
                    int prev_bottom = gs->ball_y + BALL_SIZE;
                    int prev_right  = gs->ball_x + BALL_SIZE;

                    if (prev_bottom <= b->y || gs->ball_y >= b->y + BRICK_H)
                        gs->ball_dy = -gs->ball_dy;
                    else if (prev_right <= b->x || gs->ball_x >= b->x + BRICK_W)
                        gs->ball_dx = -gs->ball_dx;
                    else
                        gs->ball_dy = -gs->ball_dy;
                }

                if (gs->bricks_alive <= 0) {
                    gs->won = 1;
                    return;
                }
                goto done_bricks;
            }
        }
    }
done_bricks:

    gs->ball_x = nx;
    gs->ball_y = ny;
}

void game_draw(const GameState *gs)
{
    int r, c, i;

    /* background */
    plat_clear(0x10, 0x10, 0x30);

    /* bricks */
    for (r = 0; r < BRICK_ROWS; r++) {
        for (c = 0; c < BRICK_COLS; c++) {
            const Brick *b = &gs->bricks[r][c];
            if (b->alive) {
                plat_draw_rect(b->x, b->y, BRICK_W, BRICK_H,
                               b->r, b->g, b->b);
            }
        }
    }

    /* paddle — white */
    plat_draw_rect(gs->paddle_x, PADDLE_Y, PADDLE_W, PADDLE_H,
                   0xFF, 0xFF, 0xFF);

    /* ball — bright yellow */
    plat_draw_rect(gs->ball_x, gs->ball_y, BALL_SIZE, BALL_SIZE,
                   0xFF, 0xFF, 0x00);

    /* lives indicator: small squares in top-right */
    for (i = 0; i < gs->lives; i++) {
        plat_draw_rect(SCREEN_W - 12 - i * 10, 4, 6, 6,
                       0xFF, 0x44, 0x44);
    }

    /* game over / won overlay: simple colored bar */
    if (gs->game_over) {
        plat_draw_rect(SCREEN_W / 2 - 50, SCREEN_H / 2 - 8, 100, 16,
                       0xFF, 0x00, 0x00);
    }
    if (gs->won) {
        plat_draw_rect(SCREEN_W / 2 - 50, SCREEN_H / 2 - 8, 100, 16,
                       0x00, 0xFF, 0x00);
    }

    plat_flip();
}
