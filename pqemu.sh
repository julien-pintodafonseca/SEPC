#!/bin/bash
make clean
make
qemu-system-i386 -s -S -m 256M -kernel kernel/kernel.bin
