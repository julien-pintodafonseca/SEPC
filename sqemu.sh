#!/bin/bash
make clean
make
qemu-system-i386 -m 256M -kernel kernel/kernel.bin
