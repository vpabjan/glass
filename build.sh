#!/bin/sh
set -e



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

echo "[*] Building Glass..."
time $CC -o glass glass.c -lX11 $FLAGS $LFLAGS


echo "[*] Building glassbg..."
time $CC -o glassbg glassbg.c -lImlib2 -lX11 $FLAGS $LFLAGS

echo "[✓] Build OK"
