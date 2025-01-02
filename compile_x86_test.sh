#!/bin/bash
mkdir deb/usr/local/bin
/bin/x86_64-pc-linux-gnu-gcc -o deb/usr/local/bin/wiibuntu wiibuntu.c -lX11 -lpng

