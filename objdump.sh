#!/bin/bash
objdump -D kernel/kernel.bin > /tmp/kernel.s && emacs /tmp/kernel.s --eval="(progn (asm-mode) (read-only-mode))" && rm /tmp/kernel.s
