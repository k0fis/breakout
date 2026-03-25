/* gen_whdload_info.c — Generate a WHDLoad .info icon for Breakout */
/* Like gen_info.c but with DefaultTool=C:WHDLoad and SLAVE tooltype */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

static void w16(FILE *f, uint16_t v) { v = htons(v); fwrite(&v, 2, 1, f); }
static void w32(FILE *f, uint32_t v) { v = htonl(v); fwrite(&v, 2, 2, f); }
static void w8(FILE *f, uint8_t v) { fwrite(&v, 1, 1, f); }

/* Write a length-prefixed string (ULONG len including NUL, then bytes) */
static void wstr(FILE *f, const char *s) {
    uint32_t len = (uint32_t)strlen(s) + 1;
    w32(f, len);
    fwrite(s, 1, len, f);
}

/* Same 24x24 Breakout icon as gen_info.c */
static const uint8_t plane0[96] = {
    0x00,0x00,0x00,0x00,
    0x7F,0xFF,0xFF,0x00, 0x7F,0xFF,0xFF,0x00,
    0x70,0x00,0x07,0x00, 0x70,0x00,0x07,0x00,
    0x70,0x00,0x07,0x00, 0x70,0x00,0x07,0x00,
    0x70,0x3C,0x07,0x00, 0x70,0x3C,0x07,0x00,
    0x70,0x00,0x07,0x00,
    0x70,0x7E,0x07,0x00, 0x70,0x7E,0x07,0x00,
    0x70,0x00,0x07,0x00,
    0x70,0xFF,0x07,0x00, 0x70,0xFF,0x07,0x00,
    0x70,0x00,0x07,0x00, 0x70,0x00,0x07,0x00,
    0x70,0x18,0x07,0x00, 0x70,0x18,0x07,0x00,
    0x70,0x00,0x07,0x00,
    0x71,0xFF,0x87,0x00,
    0x70,0x00,0x07,0x00,
    0x7F,0xFF,0xFF,0x00, 0x7F,0xFF,0xFF,0x00,
};

static const uint8_t plane1[96] = {
    0xFF,0xFF,0xFF,0x00,
    0xFF,0xFF,0xFF,0x00, 0x80,0x00,0x01,0x00,
    0x80,0x00,0x01,0x00, 0x80,0x00,0x01,0x00,
    0x80,0x00,0x01,0x00, 0x80,0x00,0x01,0x00,
    0x80,0x3C,0x01,0x00, 0x80,0x3C,0x01,0x00,
    0x80,0x00,0x01,0x00,
    0x80,0x7E,0x01,0x00, 0x80,0x7E,0x01,0x00,
    0x80,0x00,0x01,0x00,
    0x80,0xFF,0x01,0x00, 0x80,0xFF,0x01,0x00,
    0x80,0x00,0x01,0x00, 0x80,0x00,0x01,0x00,
    0x80,0x18,0x01,0x00, 0x80,0x18,0x01,0x00,
    0x80,0x00,0x01,0x00,
    0x81,0xFF,0x81,0x00,
    0x80,0x00,0x01,0x00,
    0x80,0x00,0x01,0x00, 0xFF,0xFF,0xFF,0x00,
};

int main(void) {
    const char *outpath = "assets/breakout_whdload.info";
    FILE *f = fopen(outpath, "wb");
    if (!f) { perror("fopen"); return 1; }

    /* ToolTypes to embed */
    const char *tooltypes[] = { "SLAVE=Breakout.Slave", "PRELOAD" };
    int num_tt = 2;

    /*--- DiskObject (78 bytes) ---*/
    w16(f, 0xE310);    /* do_Magic */
    w16(f, 1);         /* do_Version */

    /* Embedded Gadget (44 bytes) */
    w32(f, 0);         /* ga_Next */
    w16(f, 0);         /* ga_LeftEdge */
    w16(f, 0);         /* ga_TopEdge */
    w16(f, 24);        /* ga_Width */
    w16(f, 24);        /* ga_Height */
    w16(f, 0x0004);    /* ga_Flags: GADGIMAGE */
    w16(f, 0x0003);    /* ga_Activation */
    w16(f, 0x0001);    /* ga_GadgetType: BOOLGADGET */
    w32(f, 1);         /* ga_GadgetRender: non-NULL */
    w32(f, 0);         /* ga_SelectRender: NULL */
    w32(f, 0);         /* ga_GadgetText */
    w32(f, 0);         /* ga_MutualExclude */
    w32(f, 0);         /* ga_SpecialInfo */
    w16(f, 0);         /* ga_GadgetID */
    w32(f, 1);         /* ga_UserData (revision) */

    /* DiskObject fields continued */
    w8(f, 4);          /* do_Type: WBPROJECT (launched via DefaultTool) */
    w8(f, 0);          /* padding */
    w32(f, 1);         /* do_DefaultTool: non-NULL */
    w32(f, 1);         /* do_ToolTypes: non-NULL */
    w32(f, 0x80000000);/* do_CurrentX: NO_ICON_POSITION */
    w32(f, 0x80000000);/* do_CurrentY: NO_ICON_POSITION */
    w32(f, 0);         /* do_DrawerData */
    w32(f, 0);         /* do_ToolWindow */
    w32(f, 8192);      /* do_StackSize */

    /*--- Image structure (20 bytes) ---*/
    w16(f, 0);         /* im_LeftEdge */
    w16(f, 0);         /* im_TopEdge */
    w16(f, 24);        /* im_Width */
    w16(f, 24);        /* im_Height */
    w16(f, 2);         /* im_Depth */
    w32(f, 1);         /* im_ImageData: non-NULL */
    w8(f, 0x03);       /* im_PlanePick */
    w8(f, 0x00);       /* im_PlaneOnOff */
    w32(f, 0);         /* im_Next */

    /*--- Image data ---*/
    fwrite(plane0, 1, 96, f);
    fwrite(plane1, 1, 96, f);

    /*--- DefaultTool ---*/
    wstr(f, "C:WHDLoad");

    /*--- ToolTypes ---*/
    /* Array header: (num_entries + 1) * 4 = size of the STRPTR array in memory */
    w32(f, (uint32_t)(num_tt + 1) * 4);
    int i;
    for (i = 0; i < num_tt; i++) {
        wstr(f, tooltypes[i]);
    }

    fclose(f);
    printf("Created %s\n", outpath);
    return 0;
}
