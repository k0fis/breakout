#!/bin/sh
# Breakout — self-update script (macOS / Linux)
# Downloads the latest release from GitHub and replaces the local copy.
# Dependencies: curl, grep, cut (POSIX tools only — no jq)

set -e

REPO="k0fis/breakout"
API_URL="https://api.github.com/repos/${REPO}/releases/latest"

# --- Detect platform --------------------------------------------------------

OS="$(uname -s)"
case "$OS" in
    Darwin) ASSET_NAME="breakout-macos-arm64.zip" ;;
    Linux)  ASSET_NAME="Breakout-linux-x86_64.AppImage" ;;
    *)
        echo "Error: unsupported OS '$OS' (expected Darwin or Linux)" >&2
        exit 1
        ;;
esac

echo "Platform: $OS"
echo "Looking for asset: $ASSET_NAME"

# --- Query GitHub API --------------------------------------------------------

echo "Fetching latest release info..."
RELEASE_JSON="$(curl -sfL "$API_URL")" || {
    echo "Error: cannot reach GitHub API. Check your network connection." >&2
    exit 1
}

# Extract tag name (version)
TAG="$(echo "$RELEASE_JSON" | grep '"tag_name"' | cut -d '"' -f 4)"
if [ -z "$TAG" ]; then
    echo "Error: no releases found for $REPO." >&2
    exit 1
fi
echo "Latest version: $TAG"

# Extract download URL for our asset
DOWNLOAD_URL="$(echo "$RELEASE_JSON" | grep '"browser_download_url"' | grep "$ASSET_NAME" | cut -d '"' -f 4)"
if [ -z "$DOWNLOAD_URL" ]; then
    echo "Error: asset '$ASSET_NAME' not found in release $TAG." >&2
    exit 1
fi

# --- Download ----------------------------------------------------------------

TMPDIR_DL="$(mktemp -d)"
trap 'rm -rf "$TMPDIR_DL"' EXIT

echo "Downloading $ASSET_NAME..."
curl -fL -o "$TMPDIR_DL/$ASSET_NAME" "$DOWNLOAD_URL" || {
    echo "Error: download failed." >&2
    exit 1
}

# --- Install -----------------------------------------------------------------

case "$OS" in
    Darwin)
        echo "Extracting Breakout.app..."
        # Remove old app bundle if present
        if [ -d "Breakout.app" ]; then
            rm -rf "Breakout.app"
        fi
        unzip -qo "$TMPDIR_DL/$ASSET_NAME" -d .
        echo "Done! Breakout.app updated to $TAG."
        ;;
    Linux)
        echo "Installing AppImage..."
        cp "$TMPDIR_DL/$ASSET_NAME" "./Breakout-linux-x86_64.AppImage"
        chmod +x "./Breakout-linux-x86_64.AppImage"
        echo "Done! Breakout-linux-x86_64.AppImage updated to $TAG."
        ;;
esac
