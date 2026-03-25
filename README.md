# Breakout

Klasický Breakout/Arkanoid klon v čistém C99 s multiplatformní architekturou.
Jedna herní logika, dva grafické backendy: **SDL2** (macOS, Linux, Windows)
a **Amiga nativní** (A1200, AGA chipset).

![Build](https://img.shields.io/github/actions/workflow/status/k0fis/breakout/build.yml?branch=main)
![Platform](https://img.shields.io/badge/platform-Linux%20|%20macOS%20|%20Windows%20|%20AmigaOS-blue)
![Language](https://img.shields.io/badge/language-C99-orange)
![Target](https://img.shields.io/badge/Amiga-A1200%20(68020)-red)

## Architektura

```
src/
├── game.h / game.c          # Herní logika — čistý C99, žádné platform volání
├── platform.h                # Abstraktní platform rozhraní
├── platform_sdl2.c           # SDL2 backend (macOS / Linux / Windows)
├── platform_amiga.c          # Amiga backend (Intuition, AGA, HW joystick)
└── main.c                    # Game loop — sdílený všemi platformami
```

Herní logika (`game.c`) nezávisí na žádné platformě. Grafika, vstup a timing
jdou přes `platform.h`, který má dvě implementace.

## Podporované platformy

| Platforma | Toolchain | Backend |
|-----------|-----------|---------|
| Linux x86_64 | gcc + SDL2 | `platform_sdl2.c` |
| macOS ARM64 | clang + SDL2 | `platform_sdl2.c` |
| Windows x86_64 | MinGW + SDL2 | `platform_sdl2.c` |
| AmigaOS 3.x (68020) | vbcc (Docker) | `platform_amiga.c` |

## Build

### Linux

```bash
sudo apt install libsdl2-dev
make -f Makefile.sdl2 run
```

### macOS

```bash
brew install sdl2
make -f Makefile.sdl2 run

# Vytvořit Breakout.app (double-click z Finderu):
make -f Makefile.sdl2 bundle
open Breakout.app
```

### Windows (MSYS2)

```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2
make -f Makefile.sdl2 run
```

### Amiga (cross-compile přes Docker)

```bash
colima start                  # nebo jiný Docker runtime
make -f Makefile.amiga
```

Výstup: `build/breakout` — AmigaOS Hunk executable pro 68020+.

Používá Docker image [`walkero/docker4amigavbcc:latest-m68k`](https://github.com/walkero-gr/docker4AmigaVBCC)
s vbcc 0.9h, vasm 1.9f, vlink, NDK 3.9.

## CI / CD — GitHub Actions

Každý push a pull request automaticky buildí **všechny 4 platformy** paralelně:

| Job | Runner | Výstup |
|-----|--------|--------|
| Amiga | `ubuntu-latest` + Docker vbcc | `breakout` + `breakout.info` (Workbench ikona) |
| WHDLoad | `ubuntu-latest` + Docker vbcc + vasm | `Breakout.slave` + binárka + ikona (WHDLoad balíček) |
| Linux | `ubuntu-latest` + gcc | `Breakout-linux-x86_64.AppImage` (portable) |
| macOS | `macos-latest` + clang | `Breakout.app` (staticky linkovaný, .app bundle) |
| Windows | `windows-latest` + MSYS2/MinGW | `breakout-windows-x86_64.exe` (s ikonou) + `SDL2.dll` |

Buildy jsou ke stažení jako **artifacts** v záložce Actions.

### Release

Push tagu vytvoří automatický **GitHub Release** se všemi binárkami:

```bash
git tag v0.1
git push --tags
```

Release obsahuje:
- `breakout-amiga-68020.tar.gz` (binárka + Workbench ikona)
- `breakout-whdload.tar.gz` (WHDLoad balíček — slave + binárka + ikona)
- `Breakout-linux-x86_64.AppImage` (portable, double-click spuštění)
- `breakout-macos-arm64.zip` (.app bundle + update skript)
- `breakout-windows-x86_64.zip` (.exe s ikonou + SDL2.dll + update skript)
- `update.sh` (standalone update skript pro Linux)

## Aktualizace

Každý release obsahuje update skript, který stáhne nejnovější verzi z GitHubu:

### macOS / Linux

```bash
./update.sh
```

Skript detekuje OS (`uname -s`), stáhne správný asset a nahradí lokální soubory.
Na macOS je přibalen v zipu, na Linuxu ke stažení jako standalone `update.sh` z releasu.

### Windows (PowerShell)

```powershell
.\update.ps1
```

Přibalen v zipu vedle `.exe`.

## Ovládání

| Akce | PC klávesnice | Amiga klávesnice | Amiga joystick |
|------|---------------|------------------|----------------|
| Vlevo | ← / A | ← | Joy left |
| Vpravo | → / D | → | Joy right |
| Vypustit míček | Space | Space | Fire |
| Restart (po game over) | Space | Space | Fire |
| Ukončit | Escape | Escape | — |

## Herní parametry

- **Rozlišení:** 320×256 (PAL lowres), na PC 2× upscale (640×512)
- **Barvy:** 16 (4 bitplanes), AGA paleta s 24-bit přesností
- **Cihličky:** 10×5 = 50, pět barevných řad (50/40/30/20/10 bodů)
- **Životy:** 3
- **FPS:** 60 (SDL2 vsync) / 50 (Amiga PAL WaitTOF)

## Požadavky

**PC (SDL2 build):**
- C99 kompatibilní compiler (gcc, clang, MinGW)
- SDL2 development libraries

**Amiga cross-build:**
- Docker s image `walkero/docker4amigavbcc:latest-m68k`

**Amiga runtime:**
- AmigaOS 3.1+ (Kickstart 39+)
- 68020+ CPU
- 512 KB Chip RAM

## Struktura projektu

```
breakout/
├── .github/workflows/build.yml  # CI: multiplatformní build + release
├── src/                          # Zdrojové kódy
│   ├── Info.plist                # macOS .app bundle metadata
│   ├── app.rc                    # Windows resource file (ikona v .exe)
│   ├── breakout.desktop          # Linux .desktop entry (pro AppImage)
│   └── whdload/
│       └── Breakout.asm          # WHDLoad slave (68k asm, BOOTDOS)
├── assets/
│   ├── breakout.info             # Amiga Workbench ikona
│   ├── icon_256.png              # Ikona 256x256 (Linux AppImage)
│   └── icon.ico                  # Ikona multi-size (Windows .exe)
├── scripts/
│   ├── update.sh                 # Self-update skript (macOS/Linux)
│   └── update.ps1                # Self-update skript (Windows)
├── tools/
│   ├── gen_info.c                # Generátor Amiga .info souboru
│   └── gen_icons.py              # Generátor PNG + ICO ikon
├── build/                        # Amiga výstup (gitignored)
├── Makefile.sdl2                 # PC build (Linux/macOS/Windows)
├── Makefile.amiga                # Amiga cross-build (Docker/vbcc)
└── README.md
```

## Licence

TBD

## Další kroky

- [ ] Test v FS-UAE emulátoru
- [ ] Test na THEA1200
- [ ] Zvukové efekty (Paula)
- [ ] Textové skóre (font rendering)
- [ ] Double buffering (`ChangeScreenBuffer`)
- [ ] Více levelů
