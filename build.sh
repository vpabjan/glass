#!/bin/bash
FLAGS="-std=gnu23 \
-D_FORTIFY_SOURCE=3 \
-O2 \
-fno-plt \
-fstack-protector-strong \
-fPIE \
-flto=$(nproc) \
-fomit-frame-pointer \
-finline-functions \
-pie"

A="native" # define architecture
#A="elf_x86_64"

echo "Building Glass..."

time gcc -o glass glass.c -lX11 $FLAGS -march=$A

echo "Building glassbg..."

time gcc -o glassbg glassbg.c -lImlib2 -lX11 $FLAGS -march=$A

echo "OK"

#gcc -o glass-settings glass-settings.c -lncurses $FLAGS
