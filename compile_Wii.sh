#!/bin/bash
mkdir deb/usr/local/bin
/bin/powerpc-linux-gnu-gcc -o deb/usr/local/bin/wiibuntu wiibuntu.c -lX11 -lpng

