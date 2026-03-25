#!/bin/sh
# Download latest Amiga release of Breakout from GitHub.
# Downloads both the native binary and the WHDLoad package.
# Dependencies: curl, grep, cut (POSIX tools only)

set -e

REPO="k0fis/breakout"
API_URL="https://api.github.com/repos/${REPO}/releases/latest"

echo "Fetching latest release info..."
RELEASE_JSON="$(curl -sfL "$API_URL")" || {
    echo "Error: cannot reach GitHub API. Check your network connection." >&2
    exit 1
}

TAG="$(echo "$RELEASE_JSON" | grep '"tag_name"' | cut -d '"' -f 4)"
if [ -z "$TAG" ]; then
    echo "Error: no releases found for $REPO." >&2
    exit 1
fi
echo "Latest version: $TAG"
echo

# --- Download native Amiga binary ---

AMIGA_ASSET="breakout-amiga-68020.tar.gz"
AMIGA_URL="$(echo "$RELEASE_JSON" | grep '"browser_download_url"' | grep "$AMIGA_ASSET" | cut -d '"' -f 4)"

if [ -n "$AMIGA_URL" ]; then
    echo "Downloading $AMIGA_ASSET..."
    curl -fL -o "$AMIGA_ASSET" "$AMIGA_URL"
    echo "  -> $AMIGA_ASSET (extract: tar xzf $AMIGA_ASSET)"
else
    echo "Warning: $AMIGA_ASSET not found in release $TAG" >&2
fi

# --- Download WHDLoad package ---

WHD_ASSET="breakout-whdload.tar.gz"
WHD_URL="$(echo "$RELEASE_JSON" | grep '"browser_download_url"' | grep "$WHD_ASSET" | cut -d '"' -f 4)"

if [ -n "$WHD_URL" ]; then
    echo "Downloading $WHD_ASSET..."
    curl -fL -o "$WHD_ASSET" "$WHD_URL"
    echo "  -> $WHD_ASSET (extract: tar xzf $WHD_ASSET)"
else
    echo "Warning: $WHD_ASSET not found in release $TAG" >&2
fi

echo
echo "Done! Files downloaded for Breakout $TAG."
echo
echo "Native Amiga:"
echo "  tar xzf $AMIGA_ASSET"
echo "  Copy 'breakout' + 'breakout.info' to your Amiga hard drive."
echo
echo "WHDLoad:"
echo "  tar xzf $WHD_ASSET"
echo "  Copy contents to Games/Breakout/ on your Amiga."
echo "  Requires WHDLoad + Kickstart 3.1 ROM (kick40068.A1200)."
