#!/bin/bash
FLAGS="-std=c23 \
-D_FORTIFY_SOURCE=2 \
-O2 \
-march=native \
-fno-plt \
-fstack-protector-strong \
-fPIE \
-pie"

gcc -o glass glass.c -lX11 $FLAGS
#gcc -o glass-config.c -lncurses $FLAGS
