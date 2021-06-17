#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"
#include "stdio.h"
#include "mem.h"

#include "test/affichage-test.c"
#include "test/processus-test1.c"
#include "test/processus-test2.c"
#include "test/test1.c"
#include "test/test2.c"
#include "test/test3.c"
#include "test/test4.c"
#include "test/test5.c"
#include "test/test6.c"
#include "test/test7.c"

bool is_timer_printed = 1;

void kernel_start(void)
{
	/* variables de tests */
	bool affichageT = 0;
	bool processusT1 = 0;
	bool processusT2 = 0;

	/* initialisation */
	//call_debugger();      			  // useless with qemu -s -S
	unsigned long quartz, ticks;
	clock_settings(&quartz, &ticks);	  // réglage de l'horloge
	masque_IRQ(0, 0);					  // démasquage de l'IRQ 0
	init_traitant_IT(32, traitant_IT_32); // initialisation du traitant 32
	efface_ecran();						  // efface l'écran

	/* tests */
	if (affichageT)
		affichageTest();
	if (processusT1)
		processusTest1();
	if (processusT2)
		processusTest2();

	void idle(void)
	{
		if (start((int (*)(void *))(test6), 512, 128, "test6", NULL) == -1)
			printf("erreur start test6\n");

		// boucle d'attente
		while (1)
		{
			sti();
			hlt();
			cli();
		}
	}

	for (int i = 0; i < NBPROC; i++)
	{
		procs[i].pid = -1;
	}
	procs[0].pid = 0;
	sprintf(procs[0].nom, "%p", "idle");
	procs[0].etat = ACTIF;
	procs[0].prio = 0;

    //init pile
    procs[0].taille_pile = 512 + 64 * sizeof(int);
    procs[0].pile = mem_alloc(procs[0].taille_pile);
    int index_int = procs[0].taille_pile / 4;
    procs[0].pile[index_int - 3] = (int)(idle);
    procs[0].zone_sauv[1] = (int)(&procs[0].pile[index_int - 3]);

	procs[0].parent = -1;
	for (int n = 0; n < NBPROC; n++)
	{
		procs[0].fils[n] = -1;
	}
	file_procs[0] = &procs[0];

	idle();
	return;
}
