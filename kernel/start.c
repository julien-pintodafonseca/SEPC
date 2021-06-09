#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"

#include "test/affichage-test.c"
#include "horloge.h"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n - 1);
}

void kernel_start(void)
{
	// call_debugger(); // useless with qemu -s -S
	init_traitant_IT(32, tic_PIT); // initialisation traitant 32
	masque_IRQ(0, 1);			   // masque l'IRQ 0

	/* AFFICHAGE.C */
	/*
	for (int j = 0 ; j < 40 ; j++) {
	    printf("abcde");
	}
	*/

	// affichageTest();

	/* HORLOGE.C */
	// démasquage des interruptions externes
	sti();

	while (1)
		hlt();
	return;
}
