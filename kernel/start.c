#include "debugger.h"
#include "cpu.h"
#include "printf.c"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}


void kernel_start(void)
{
	int i;
	// call_debugger(); // useless with qemu -s -S

	i = 10;

	i = fact(i);
    //for (int j = 0 ; j < 40 ; j++) {
    //    printf("abcde");
    //}

    while(1)
	  hlt();

	return;
}
