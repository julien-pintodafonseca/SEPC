#!/bin/bash
make clean
make
qemu-system-i386 -m 64M -kernel kernel/kernel.bin
