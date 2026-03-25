#!/usr/bin/env python3
"""Generate breakout icon files: PNG (256x256) and ICO (multi-size) for Windows.

Uses only Python 3 standard library (struct, zlib). Run from project root:
    python3 tools/gen_icons.py
"""
import struct
import zlib
import os

# --- Colors (R, G, B, A) ---
BG         = (26, 26, 46, 255)
BORDER     = (220, 220, 240, 255)
BRICK_ROWS = [
    (231, 76, 60, 255),     # red
    (230, 126, 34, 255),    # orange
    (241, 196, 15, 255),    # yellow
    (46, 204, 113, 255),    # green
    (52, 152, 219, 255),    # blue
]
WHITE = (255, 255, 255, 255)

# --- Icon drawing (256x256) ---

def create_icon_256():
    W, H = 256, 256
    px = [[BG] * W for _ in range(H)]

    # 1px border around play area (inset 12px)
    m = 12
    for x in range(m, W - m):
        px[m][x] = BORDER
        px[H - m - 1][x] = BORDER
    for y in range(m, H - m):
        px[y][m] = BORDER
        px[y][W - m - 1] = BORDER

    # Bricks: 5 rows x 8 cols
    bw, bh, gap = 25, 16, 3
    total_w = 8 * bw + 7 * gap          # 221
    ox = (W - total_w) // 2              # left offset to center
    oy = 32                              # top of first row

    for row in range(5):
        color = BRICK_ROWS[row]
        # subtle 3D: lighter top edge, darker bottom edge
        top_edge = tuple(min(c + 40, 255) for c in color[:3]) + (255,)
        bot_edge = tuple(max(c - 50, 0) for c in color[:3]) + (255,)
        by = oy + row * (bh + gap)
        for col in range(8):
            bx = ox + col * (bw + gap)
            for y in range(by, by + bh):
                for x in range(bx, bx + bw):
                    if y == by:
                        px[y][x] = top_edge
                    elif y == by + bh - 1:
                        px[y][x] = bot_edge
                    else:
                        px[y][x] = color

    # Ball (filled circle)
    bcx, bcy, br = W // 2, 172, 7
    for y in range(bcy - br - 1, bcy + br + 2):
        for x in range(bcx - br - 1, bcx + br + 2):
            if 0 <= x < W and 0 <= y < H:
                if (x - bcx) ** 2 + (y - bcy) ** 2 <= br * br:
                    px[y][x] = WHITE

    # Paddle
    pw, ph = 72, 10
    ppx = (W - pw) // 2
    ppy = 218
    for y in range(ppy, ppy + ph):
        for x in range(ppx, ppx + pw):
            px[y][x] = WHITE

    return W, H, px


def downscale(px, sw, sh, dw, dh):
    """Nearest-neighbor downscale."""
    out = []
    for y in range(dh):
        row = []
        sy = y * sh // dh
        for x in range(dw):
            sx = x * sw // dw
            row.append(px[sy][sx])
        out.append(row)
    return out


# --- PNG encoder (RGBA, 8-bit) ---

def _png_chunk(tag, data):
    c = tag + data
    crc = zlib.crc32(c) & 0xFFFFFFFF
    return struct.pack('>I', len(data)) + c + struct.pack('>I', crc)


def encode_png(w, h, px):
    raw = b''
    for row in px:
        raw += b'\x00'                       # filter: None
        for r, g, b, a in row:
            raw += struct.pack('BBBB', r, g, b, a)

    sig = b'\x89PNG\r\n\x1a\n'
    ihdr = struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0)
    return (sig
            + _png_chunk(b'IHDR', ihdr)
            + _png_chunk(b'IDAT', zlib.compress(raw, 9))
            + _png_chunk(b'IEND', b''))


# --- ICO encoder (PNG-in-ICO, modern format) ---

def encode_ico(entries):
    """entries: list of (width, height, png_bytes)"""
    n = len(entries)
    hdr = struct.pack('<HHH', 0, 1, n)      # reserved, type=ICO, count
    offset = 6 + n * 16                      # after header + directory

    directory = b''
    for w, h, png in entries:
        iw = 0 if w >= 256 else w
        ih = 0 if h >= 256 else h
        directory += struct.pack('<BBBBHHII',
                                 iw, ih, 0, 0, 1, 32, len(png), offset)
        offset += len(png)

    return hdr + directory + b''.join(png for _, _, png in entries)


# --- Main ---

def main():
    os.makedirs('assets', exist_ok=True)

    w, h, px = create_icon_256()

    sizes = [256, 48, 32, 16]
    pngs = {}
    for s in sizes:
        scaled = px if s == 256 else downscale(px, w, h, s, s)
        pngs[s] = encode_png(s, s, scaled)

    # PNG for Linux (AppImage / desktop)
    path_png = 'assets/icon_256.png'
    with open(path_png, 'wb') as f:
        f.write(pngs[256])
    print(f'Created {path_png} ({len(pngs[256])} bytes)')

    # ICO for Windows (multi-size)
    ico = encode_ico([(s, s, pngs[s]) for s in sizes])
    path_ico = 'assets/icon.ico'
    with open(path_ico, 'wb') as f:
        f.write(ico)
    print(f'Created {path_ico} ({len(ico)} bytes)')


if __name__ == '__main__':
    main()
