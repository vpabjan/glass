#!/bin/bash

# download, build glass
git clone https://github.com/vpabjan/glass.git
cd glass
chmod +x build.sh
./build.sh

if command -v doas >/dev/null 2>&1; then
    ASROOT="doas"
elif command -v sudo >/dev/null 2>&1; then
    ASROOT="sudo"
else
    echo "Neither doas nor sudo found" >&2
    exit 1
fi

$ASROOT ./install.sh
