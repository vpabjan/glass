#!/bin/bash
gcc -o glass glass.c -lX11 -std=c23 -D_FORTIFY_SOURCE=2 -O2 -fstack-protector
