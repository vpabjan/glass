#!/bin/sh
set -e

# decide prefix
if [ "$(id -u)" -eq 0 ]; then
    PREFIX=${PREFIX:-/usr/local}
else
    PREFIX=${PREFIX:-$HOME/.local}
fi

BIN="$PREFIX/bin"
LIB="$PREFIX/lib/glass"
SHARE="$PREFIX/share/glass"
ETC="$PREFIX/etc/glass"
XSESSIONS="$PREFIX/share/xsessions"

echo "[*] Installing glass"
echo "    PREFIX=$PREFIX"

# sanity
[ -x ./glass ] || {
    echo "glass binary not found (run ./build.sh first)"
    exit 1
}

# dirs
install -d \
    "$BIN" \
    "$LIB" \
    "$SHARE" \
    "$ETC" \
    "$XSESSIONS"

# binaries
install -m755 glass "$BIN/glass"
install -m755 glassbg "$BIN/glassbg"

# config (install once)
if [ ! -f "$ETC/glass.conf" ]; then
    install -m644 extra/glass.conf "$ETC/glass.conf"
else
    echo "    glass.conf exists, not overwriting"
fi

# assets
install -m644 extra/wallpaper.jpg "$SHARE/wallpaper.jpg"
install -m644 extra/xinitrc "$SHARE/xinitrc"
install -m644 version "$SHARE/version"

# scripts
#install -m755 extra/start-glass "$LIB/start-glass"
#install -m755 extra/update.sh "$LIB/update.sh"

# session (optional)
if [ -d "$XSESSIONS" ]; then
    install -m644 extra/glass.desktop "$XSESSIONS/glass.desktop"
fi

echo "[✓] Install complete"
