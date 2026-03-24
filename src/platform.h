#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

/* Initialize platform: open window at w x h pixels. Returns 0 on success. */
int  plat_init(int w, int h, const char *title);

/* Shut down and free resources. */
void plat_shutdown(void);

/* Clear the screen to a solid color. */
void plat_clear(uint8_t r, uint8_t g, uint8_t b);

/* Draw a filled rectangle. */
void plat_draw_rect(int x, int y, int w, int h,
                    uint8_t r, uint8_t g, uint8_t b);

/* Present the back-buffer to the screen. */
void plat_flip(void);

/* Poll input.  Returns 0 normally, 1 if quit requested.
   *left / *right = 1 when the corresponding direction is held.
   *fire = 1 when fire button pressed (space / joystick button). */
int  plat_poll_input(int *left, int *right, int *fire);

/* Sleep for ms milliseconds. */
void plat_delay(int ms);

/* Return milliseconds since plat_init(). */
uint32_t plat_ticks(void);

#endif /* PLATFORM_H */
