/*
 * platform_amiga.c — Amiga backend for Breakout
 *
 * Target: A1200 (68020, AGA, AmigaOS 3.1)
 * Uses: Intuition screen, graphics.library RastPort, timer.device,
 *       hardware joystick reading (port 1)
 */

#include "platform.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <devices/timer.h>
#include <hardware/custom.h>
#include <hardware/cia.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/timer.h>

/* ---- Library bases (proto headers declare them extern) ---- */

struct IntuitionBase *IntuitionBase = NULL;
struct GfxBase       *GfxBase       = NULL;
struct Device        *TimerBase     = NULL;

/* Hardware registers for joystick reading */
extern struct Custom custom;
extern struct CIA ciaa;

/* ---- Color palette ---- */

#define NUM_PENS 12

static struct { UBYTE r, g, b; } pen_table[NUM_PENS] = {
    {0x10, 0x10, 0x30},  /*  0: background (dark blue) */
    {0xFF, 0x22, 0x22},  /*  1: red brick */
    {0xFF, 0x88, 0x00},  /*  2: orange brick */
    {0xFF, 0xDD, 0x00},  /*  3: yellow brick */
    {0x00, 0xCC, 0x44},  /*  4: green brick */
    {0x22, 0x88, 0xFF},  /*  5: blue brick */
    {0xFF, 0xFF, 0xFF},  /*  6: white (paddle) */
    {0xFF, 0xFF, 0x00},  /*  7: yellow (ball) */
    {0xFF, 0x44, 0x44},  /*  8: light red (lives) */
    {0xFF, 0x00, 0x00},  /*  9: red (game over bar) */
    {0x00, 0xFF, 0x00},  /* 10: green (won bar) */
    {0x00, 0x00, 0x00},  /* 11: black (spare) */
};

/* Convert 8-bit value to LoadRGB32 left-justified 32-bit fraction */
#define RGB8TO32(x) ((ULONG)(x) * 0x01010101UL)

/* ---- Globals ---- */

static struct Screen     *scr;
static struct Window     *win;
static struct RastPort   *rp;

static struct MsgPort    *timer_port;
static struct timerequest *timer_io;

static UBYTE keystate[128];

/* ---- Helpers ---- */

static int find_pen(UBYTE r, UBYTE g, UBYTE b)
{
    int i;
    for (i = 0; i < NUM_PENS; i++) {
        if (pen_table[i].r == r &&
            pen_table[i].g == g &&
            pen_table[i].b == b)
            return i;
    }
    return 0;
}

/* Read digital joystick in port 1 */
static void read_joy(int *left, int *right, int *fire)
{
    UWORD joy = custom.joy1dat;

    /* Standard gray-code decoding for digital joystick */
    *right = ((joy >> 1) ^ joy) & 1;
    *left  = ((joy >> 9) ^ (joy >> 8)) & 1;
    *fire  = !(ciaa.ciapra & 0x0080);  /* fire button, active low */
}

/* ---- Platform API ---- */

int plat_init(int w, int h, const char *title)
{
    ULONG colortable[1 + NUM_PENS * 3 + 1];
    int i;

    /* Open required libraries */
    IntuitionBase = (struct IntuitionBase *)
        OpenLibrary("intuition.library", 39);
    if (!IntuitionBase) return -1;

    GfxBase = (struct GfxBase *)
        OpenLibrary("graphics.library", 39);
    if (!GfxBase) goto fail;

    /* Open custom screen: 320x256, 4 bitplanes = 16 colors */
    scr = OpenScreenTags(NULL,
        SA_Width,     w,
        SA_Height,    h,
        SA_Depth,     4,
        SA_Type,      CUSTOMSCREEN,
        SA_Quiet,     TRUE,
        SA_ShowTitle, FALSE,
        SA_Title,     (ULONG)title,
        TAG_DONE);

    if (!scr) goto fail;

    /* Load AGA palette (32-bit precision per gun) */
    colortable[0] = ((ULONG)NUM_PENS << 16) | 0;  /* count | first_pen */
    for (i = 0; i < NUM_PENS; i++) {
        colortable[1 + i * 3 + 0] = RGB8TO32(pen_table[i].r);
        colortable[1 + i * 3 + 1] = RGB8TO32(pen_table[i].g);
        colortable[1 + i * 3 + 2] = RGB8TO32(pen_table[i].b);
    }
    colortable[1 + NUM_PENS * 3] = 0;  /* terminator */
    LoadRGB32(&scr->ViewPort, colortable);

    /* Borderless window for IDCMP input */
    win = OpenWindowTags(NULL,
        WA_CustomScreen,  (ULONG)scr,
        WA_Left,          0,
        WA_Top,           0,
        WA_Width,         w,
        WA_Height,        h,
        WA_Borderless,    TRUE,
        WA_Backdrop,      TRUE,
        WA_Activate,      TRUE,
        WA_RMBTrap,       TRUE,
        WA_IDCMP,         IDCMP_RAWKEY,
        TAG_DONE);

    if (!win) goto fail;

    rp = win->RPort;

    /* Open timer.device for plat_ticks / plat_delay */
    timer_port = CreateMsgPort();
    if (!timer_port) goto fail;

    timer_io = (struct timerequest *)
        CreateIORequest(timer_port, sizeof(*timer_io));
    if (!timer_io) goto fail;

    if (OpenDevice("timer.device", UNIT_MICROHZ,
                    (struct IORequest *)timer_io, 0) != 0) {
        timer_io->tr_node.io_Device = NULL;
        goto fail;
    }
    TimerBase = timer_io->tr_node.io_Device;

    /* Clear key state */
    for (i = 0; i < 128; i++)
        keystate[i] = 0;

    return 0;

fail:
    plat_shutdown();
    return -1;
}

void plat_shutdown(void)
{
    if (timer_io) {
        if (timer_io->tr_node.io_Device)
            CloseDevice((struct IORequest *)timer_io);
        DeleteIORequest((struct IORequest *)timer_io);
        timer_io = NULL;
    }
    if (timer_port) {
        DeleteMsgPort(timer_port);
        timer_port = NULL;
    }
    if (win) {
        CloseWindow(win);
        win = NULL;
    }
    if (scr) {
        CloseScreen(scr);
        scr = NULL;
    }
    if (GfxBase) {
        CloseLibrary((struct Library *)GfxBase);
        GfxBase = NULL;
    }
    if (IntuitionBase) {
        CloseLibrary((struct Library *)IntuitionBase);
        IntuitionBase = NULL;
    }
    TimerBase = NULL;
}

void plat_clear(uint8_t r, uint8_t g, uint8_t b)
{
    SetAPen(rp, find_pen(r, g, b));
    RectFill(rp, 0, 0, scr->Width - 1, scr->Height - 1);
}

void plat_draw_rect(int x, int y, int w, int h,
                    uint8_t r, uint8_t g, uint8_t b)
{
    SetAPen(rp, find_pen(r, g, b));
    RectFill(rp, x, y, x + w - 1, y + h - 1);
}

void plat_flip(void)
{
    WaitTOF();   /* wait for vertical blank — simple vsync */
}

int plat_poll_input(int *left, int *right, int *fire)
{
    struct IntuiMessage *msg;
    ULONG cls;
    UWORD code;
    int jl, jr, jf;

    /* Drain IDCMP messages (keyboard) */
    while ((msg = (struct IntuiMessage *)GetMsg(win->UserPort))) {
        cls  = msg->Class;
        code = msg->Code;
        ReplyMsg((struct Message *)msg);

        if (cls == IDCMP_RAWKEY) {
            if (code < 0x80)
                keystate[code] = 1;        /* key down */
            else
                keystate[code & 0x7F] = 0; /* key up */
        }
    }

    /* Keyboard: cursor keys + space */
    *left  = keystate[0x4F];  /* cursor left */
    *right = keystate[0x4E];  /* cursor right */
    *fire  = keystate[0x40];  /* space */

    /* Merge with joystick (port 1) */
    read_joy(&jl, &jr, &jf);
    *left  |= jl;
    *right |= jr;
    *fire  |= jf;

    return keystate[0x45];    /* Escape = quit */
}

void plat_delay(int ms)
{
    timer_io->tr_node.io_Command = TR_ADDREQUEST;
    timer_io->tr_time.tv_secs  = ms / 1000;
    timer_io->tr_time.tv_micro = (ms % 1000) * 1000;
    DoIO((struct IORequest *)timer_io);
}

uint32_t plat_ticks(void)
{
    struct timeval tv;
    GetSysTime(&tv);
    return (ULONG)(tv.tv_secs * 1000 + tv.tv_micro / 1000);
}
