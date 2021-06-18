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
#include "test/test8.c"

void quit(void)
{
	exit(0);
}

struct
{
	const char *name;
	void (*f)(void);
} commands[] = {
	{"1", test1},
	{"2", test2},
	{"3", test3},
	//{"4", test4},
	{"5", test5},
	{"6", test6},
	//{"7", test7},
	//{"8", test8},
	//{"9", test9},
	//{"10", test10},
	//{"11", test11},
	//{"12", test12},
	//{"13", test13},
	//{"14", test14},
	//{"15", test15},
	//{"16", test16},
	//{"17", test17},
	//{"18", test18},
	//{"19", test19},
	//{"20", test20},
	{"q", quit},
};

void auto_test(void)
{
	int i = 0;

	while (commands[i].f != quit)
	{
		printf("Test %s : ", commands[i].name);
		commands[i++].f();
	}
}

void kernel_start(void)
{
	timer_print = 1; // mettre à 1 pour afficher le timer, 0 sinon

	/* variables de tests spécifiques */
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

	/* tests spécifiques */
	if (affichageT)
		affichageTest();
	if (processusT1)
		processusTest1();
	if (processusT2)
		processusTest2();

	void idle(void)
	{
		/* auto_test */
		start((int (*)(void *))(auto_test), 4000, 128, "auto_test", NULL);

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
	procs[0].taille_pile = 4000 + 128 * sizeof(int);
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
