#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"
#include "stdio.h"

#include "test/affichage-test.c"
#include "test/processus-test1.c"
#include "test/processus-test2.c"
#include "horloge.h"

void kernel_start(void)
{
	/* variables de tests */
	bool affichageT = 0;
	bool processusT1 = 0;
	bool processusT2 = 0;
	bool horlogeT = 1;

	/* initialisation */
	//call_debugger(); // useless with qemu -s -S
	efface_ecran(); // efface l'écran

	/* tests */
	if (affichageT)
		affichageTest();
	if (processusT1)
		processusTest1();
	if (processusT2)
		processusTest2();
	if (horlogeT)
	{
		printf("start\n");
		// initialisations
		unsigned long quartz, ticks;
		clock_settings(&quartz, &ticks);	  // réglade de l'horloge
		masque_IRQ(0, 0);					  // démasquage de l'IRQ 0
		init_traitant_IT(32, traitant_IT_32); // initialisation du traitant 32
		sti();								  // démasquage des interruptions externes
	}

	// boucle d'attente
	while (1)
		hlt();
	return;
}
