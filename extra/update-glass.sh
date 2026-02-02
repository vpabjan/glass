#!/bin/bash

USER=$(id -un)
HOST=$(uname -n)



PS4="$USER@$HOST~# "

echo "[*] Cloning glass"

set -x

cd /tmp

git clone https://github.com/vpabjan/glass.git
cd glass

{ set +x; } 2>/dev/null

echo "[*] Applying permissions"

set -x
chmod +x build.sh
chmod +x install.sh
{ set +x; } 2>/dev/null

echo "[*] Building glass"

./build.sh

if command -v doas >/dev/null 2>&1; then
    ASROOT="doas"
elif command -v sudo >/dev/null 2>&1; then
    ASROOT="sudo"
else
    echo "Neither doas nor sudo found" >&2
    exit 1
fi

echo "[*] Installing glass"

$ASROOT ./install.sh

echo "[✓] Update complete!"
