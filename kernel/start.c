#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"
#include "stdio.h"

#include "test/affichage-test.c"
#include "affichage.h"
#include "horloge.h"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n - 1);
}

void kernel_start(void)
{
	/* variables de tests */
	bool affichageT = 0;

	/* initialisation */
	// call_debugger(); 			// useless with qemu -s -S
	efface_ecran(); 				// efface l'écran
	init_traitant_IT(32, tic_PIT);	// initialisation traitant 32
	masque_IRQ(0, 1);				// masque l'IRQ 0

	/* tests */
	if (affichageT)
		affichageTest();

	// démasquage des interruptions externes
	sti(); // TODO : temporaire, à supprimer et à rajouter dans les fichiers de tests une fois ok

	// boucle d'attente
	while (1)
		hlt();
	return;
}
