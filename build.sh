#!/bin/sh

set -e

USER=$(id -un)
HOST=$(uname -n)


PS4="$USER@$HOST~# "


FLAGS="-std=gnu23 \
-D_FORTIFY_SOURCE=3 \
-O2 \
-fno-plt \
-fstack-protector-strong \
-fPIE \
-fomit-frame-pointer \
-finline-functions"

LFLAGS="
-pie \
-flto"

ARCH=${ARCH:-native}
CC=${CC:-cc}

time {
    echo "[*] Setting up build directory"
    set -x
    rm -rf build
    mkdir -p build
    { set +x; } 2>/dev/null

    echo "[*] Building Glass..."
    set -x
    $CC -o build/glass glass.c -lX11 $FLAGS $LFLAGS
    { set +x; } 2>/dev/null

    echo "[*] Building glassbg..."
    set -x
    $CC -o build/glassbg glassbg.c -lImlib2 -lX11 $FLAGS $LFLAGS
    { set +x; } 2>/dev/null

    echo "[✓] Build OK"
}
