#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/* ---- constants ---- */

#define SCREEN_W      320
#define SCREEN_H      256

#define BRICK_COLS    10
#define BRICK_ROWS    5
#define BRICK_W       28
#define BRICK_H       10
#define BRICK_GAP     2
#define BRICKS_X0     ((SCREEN_W - BRICK_COLS * (BRICK_W + BRICK_GAP) + BRICK_GAP) / 2)
#define BRICKS_Y0     30

#define PADDLE_W      48
#define PADDLE_H      8
#define PADDLE_Y      (SCREEN_H - 24)
#define PADDLE_SPEED  4

#define BALL_SIZE     4
#define BALL_SPEED    3

#define MAX_LIVES     3

/* ---- types ---- */

typedef struct {
    int x, y;
    int alive;
    uint8_t r, g, b;
} Brick;

typedef struct {
    /* paddle */
    int paddle_x;

    /* ball */
    int ball_x, ball_y;
    int ball_dx, ball_dy;
    int ball_active;        /* 0 = stuck to paddle, 1 = moving */

    /* bricks */
    Brick bricks[BRICK_ROWS][BRICK_COLS];
    int bricks_alive;

    /* state */
    int score;
    int lives;
    int game_over;
    int won;
} GameState;

/* ---- API ---- */

void game_init(GameState *gs);
void game_update(GameState *gs, int left, int right, int fire);
void game_draw(const GameState *gs);

#endif /* GAME_H */
