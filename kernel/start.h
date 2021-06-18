#ifndef __START_H__
#define __START_H__

#include "stdbool.h"

#define FIRST_STACK_SIZE 16384

#ifndef ASSEMBLER

extern char first_stack[FIRST_STACK_SIZE];

void auto_test(void);

/* This is the first user process to start. It is mapped to the entry point
of the user code. */
int user_start(void *);

/* The kernel entry point */
void kernel_start(void);

#endif

#endif
