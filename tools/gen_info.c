/* gen_info.c — Generate a minimal Amiga .info file (Tool icon) */
/* Writes a valid DiskObject with a small 24x24 2-bitplane icon */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>  /* htons, htonl — big-endian helpers */

/* Write big-endian 16-bit */
static void w16(FILE *f, uint16_t v) { v = htons(v); fwrite(&v, 2, 1, f); }
/* Write big-endian 32-bit */
static void w32(FILE *f, uint32_t v) { v = htonl(v); fwrite(&v, 2, 2, f); }
/* Write a single byte */
static void w8(FILE *f, uint8_t v) { fwrite(&v, 1, 1, f); }

/*
 * Amiga .info file layout:
 *   struct DiskObject    (78 bytes)
 *   struct Image 1       (20 bytes)
 *   Image 1 data         (plane0 + plane1)
 *   DrawerData / ToolTypes / DefaultTool (referenced by pointers)
 *
 * Icon: 24x24 pixels, 2 bitplanes (4 colors), word-aligned → 2 words/row = 4 bytes/row
 *   Each plane: 24 rows × 4 bytes = 96 bytes
 *   Total image data: 192 bytes
 */

/* 24x24 icon — simple "B" shape for Breakout */
/* Plane 0: foreground shape */
static const uint8_t plane0[96] = {
    0x00,0x00,0x00,0x00, /* row 0  - border */
    0x7F,0xFF,0xFF,0x00, /* row 1  */
    0x7F,0xFF,0xFF,0x00, /* row 2  */
    0x70,0x00,0x07,0x00, /* row 3  */
    0x70,0x00,0x07,0x00, /* row 4  */
    0x70,0x00,0x07,0x00, /* row 5  */
    0x70,0x00,0x07,0x00, /* row 6  */
    0x70,0x3C,0x07,0x00, /* row 7  - brick row */
    0x70,0x3C,0x07,0x00, /* row 8  */
    0x70,0x00,0x07,0x00, /* row 9  */
    0x70,0x7E,0x07,0x00, /* row 10 - brick row */
    0x70,0x7E,0x07,0x00, /* row 11 */
    0x70,0x00,0x07,0x00, /* row 12 */
    0x70,0xFF,0x07,0x00, /* row 13 - brick row */
    0x70,0xFF,0x07,0x00, /* row 14 */
    0x70,0x00,0x07,0x00, /* row 15 */
    0x70,0x00,0x07,0x00, /* row 16 */
    0x70,0x18,0x07,0x00, /* row 17 - ball */
    0x70,0x18,0x07,0x00, /* row 18 */
    0x70,0x00,0x07,0x00, /* row 19 */
    0x71,0xFF,0x87,0x00, /* row 20 - paddle */
    0x70,0x00,0x07,0x00, /* row 21 */
    0x7F,0xFF,0xFF,0x00, /* row 22 */
    0x7F,0xFF,0xFF,0x00, /* row 23 */
};

/* Plane 1: highlight/shadow — gives depth to the frame */
static const uint8_t plane1[96] = {
    0xFF,0xFF,0xFF,0x00, /* row 0  */
    0xFF,0xFF,0xFF,0x00, /* row 1  */
    0x80,0x00,0x01,0x00, /* row 2  */
    0x80,0x00,0x01,0x00, /* row 3  */
    0x80,0x00,0x01,0x00, /* row 4  */
    0x80,0x00,0x01,0x00, /* row 5  */
    0x80,0x00,0x01,0x00, /* row 6  */
    0x80,0x3C,0x01,0x00, /* row 7  */
    0x80,0x3C,0x01,0x00, /* row 8  */
    0x80,0x00,0x01,0x00, /* row 9  */
    0x80,0x7E,0x01,0x00, /* row 10 */
    0x80,0x7E,0x01,0x00, /* row 11 */
    0x80,0x00,0x01,0x00, /* row 12 */
    0x80,0xFF,0x01,0x00, /* row 13 */
    0x80,0xFF,0x01,0x00, /* row 14 */
    0x80,0x00,0x01,0x00, /* row 15 */
    0x80,0x00,0x01,0x00, /* row 16 */
    0x80,0x18,0x01,0x00, /* row 17 */
    0x80,0x18,0x01,0x00, /* row 18 */
    0x80,0x00,0x01,0x00, /* row 19 */
    0x81,0xFF,0x81,0x00, /* row 20 */
    0x80,0x00,0x01,0x00, /* row 21 */
    0x80,0x00,0x01,0x00, /* row 22 */
    0xFF,0xFF,0xFF,0x00, /* row 23 */
};

int main(void) {
    FILE *f = fopen("assets/breakout.info", "wb");
    if (!f) { perror("fopen"); return 1; }

    /*--- DiskObject structure (78 bytes) ---*/

    /* do_Magic: WB_DISKMAGIC = 0xE310 */
    w16(f, 0xE310);
    /* do_Version: WB_DISKVERSION = 1 */
    w16(f, 1);

    /*--- Embedded Gadget structure (44 bytes) ---*/
    /* ga_Next: NULL */
    w32(f, 0);
    /* ga_LeftEdge, ga_TopEdge */
    w16(f, 0);   /* LeftEdge */
    w16(f, 0);   /* TopEdge */
    /* ga_Width, ga_Height */
    w16(f, 24);  /* Width — matches icon */
    w16(f, 24);  /* Height */
    /* ga_Flags: GADGIMAGE | GADGHCOMP = 0x0004 | 0x0002 */
    w16(f, 0x0004);
    /* ga_Activation: RELVERIFY | GADGIMMEDIATE */
    w16(f, 0x0003);
    /* ga_GadgetType: BOOLGADGET */
    w16(f, 0x0001);
    /* ga_GadgetRender: non-NULL placeholder (will be image 1) */
    w32(f, 1);
    /* ga_SelectRender: NULL (no alternate image) */
    w32(f, 0);
    /* ga_GadgetText: NULL */
    w32(f, 0);
    /* ga_MutualExclude */
    w32(f, 0);
    /* ga_SpecialInfo: NULL */
    w32(f, 0);
    /* ga_GadgetID */
    w16(f, 0);
    /* ga_UserData: revision (1) */
    w32(f, 1);

    /*--- Back to DiskObject fields ---*/
    /* do_Type: WBTOOL = 1 */
    w8(f, 1);
    /* (padding byte) */
    w8(f, 0);
    /* do_DefaultTool: non-NULL (we'll write it) */
    w32(f, 1);
    /* do_ToolTypes: NULL */
    w32(f, 0);
    /* do_CurrentX: NO_ICON_POSITION */
    w32(f, 0x80000000);
    /* do_CurrentY: NO_ICON_POSITION */
    w32(f, 0x80000000);
    /* do_DrawerData: NULL (not a drawer) */
    w32(f, 0);
    /* do_ToolWindow: NULL */
    w32(f, 0);
    /* do_StackSize: 8192 */
    w32(f, 8192);

    /*--- Image structure (20 bytes) ---*/
    /* im_LeftEdge, im_TopEdge */
    w16(f, 0);
    w16(f, 0);
    /* im_Width, im_Height, im_Depth */
    w16(f, 24);   /* Width */
    w16(f, 24);   /* Height */
    w16(f, 2);    /* Depth (2 bitplanes = 4 colors) */
    /* im_ImageData: non-NULL placeholder */
    w32(f, 1);
    /* im_PlanePick: 0x03 (both planes) */
    w8(f, 0x03);
    /* im_PlaneOnOff: 0x00 */
    w8(f, 0x00);
    /* im_Next: NULL */
    w32(f, 0);

    /*--- Image data: plane0 then plane1 ---*/
    fwrite(plane0, 1, 96, f);
    fwrite(plane1, 1, 96, f);

    /*--- DefaultTool string ---*/
    /* Length (32-bit, including NUL) then the string */
    const char *deftool = ":breakout";
    uint32_t len = (uint32_t)strlen(deftool) + 1;
    w32(f, len);
    fwrite(deftool, 1, len, f);

    fclose(f);
    printf("Created assets/breakout.info (%ld bytes total)\n",
           78L + 20 + 192 + 4 + (long)len);
    return 0;
}
