#!/bin/bash
cd deb
mkdir usr
cd usr
mkdir local
cd local
mkdir bin
cd ../../../
/bin/x86_64-pc-linux-gnu-gcc -o deb/usr/local/bin/wiibuntu wiibuntu.c -lX11 -lpng

