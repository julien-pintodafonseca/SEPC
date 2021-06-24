#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"
#include "stdio.h"
#include "mem.h"

#include "file_messages.h"

#include "test/affichage-test.c"
#include "test/processus-test1.c"
#include "test/processus-test2.c"

#include "test/phase4.c"

#include "test/test1.c"
#include "test/test2.c"
#include "test/test3.c"
#include "test/test4.c"
#include "test/test5.c"
#include "test/test6.c"
#include "test/test7.c"
#include "test/test8.c"
#include "test/test9.c"
#include "test/test10.c"
#include "test/test11.c"
#include "test/test12.c"
#include "test/test13.c"
#include "test/test14.c"
#include "test/test15.c"
#include "test/test16.c"
#include "test/test17.c"
#include "test/test20.c"

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
	{"8", test8},
	//{"9", test9},
	{"10", test10},
	{"11", test11},
	{"12", test12},
	{"13", test13}, // NOK
	//{"14", test14}, // NOK
	{"15", test15},
	{"16", test16},
	//{"17", test17}, // NOK
	//{"18", test18}, // Il manque des fonctions
	//{"19", test19}, // Il manque des fonctions
	//{"20", test20}, // NOK
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
	/* attention à ne bien activer qu'une seule variable de test à la fois !!! */
	/* variables de tests spécifiques */
	bool affichageT = false;
	bool processusT1 = false;
	bool processusT2 = false;
	/* phase 4 - wait_clock() */
	bool phase4 = false;
	/* auto_test */
	bool autoT = true;

	/* initialisation */
	//call_debugger();      			  // useless with qemu -s -S
	efface_ecran(); // efface l'écran

	/* horloge */
	unsigned long quartz, ticks;
	clock_settings(&quartz, &ticks);	  // on récupère quartz et ticks
	outb(0x34, 0x43);					  // réglage de l'horloge
	outb(ticks % 256, 0x40);			  // ...
	outb(ticks >> 8, 0x40);				  // ...
	init_sleeping_file_procs();			  // initialise la liste des processus endormis
	init_bloque_fils_file_procs();		  // initialise la liste des processus attendant un fils
	masque_IRQ(0, 0);					  // démasquage de l'IRQ 0
	init_traitant_IT(32, traitant_IT_32); // initialisation du traitant 32
	print_timer = true;					  // affiche le timer

	/* file de messages */
	init_waiting_for_new_message_file();

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
		if (autoT)
			start((int (*)(void *))(auto_test), 4000, 128, "auto_test", NULL);

		/* phase 4*/
		if (phase4)
			start((int (*)(void *))(prog_phase4), 4000, 128, "prog_phase4", NULL);

		// boucle d'attente
		while (1)
		{
			sti();
			hlt();
			cli();
		}
	}

	initialize((int (*)(void *))(idle), "idle");
	idle();
	return;
}
