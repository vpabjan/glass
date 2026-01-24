#!/bin/bash
FLAGS="-std=gnu23 \
-D_FORTIFY_SOURCE=2 \
-O2 \
-march=native \
-fno-plt \
-fstack-protector-strong \
-fPIE \
-pie"

gcc -o glass glass.c -lX11 $FLAGS
gcc -o glassbg glassbg.c -lImlib2 -lX11 $FLAGS
#gcc -o glass-settings glass-settings.c -lncurses $FLAGS
