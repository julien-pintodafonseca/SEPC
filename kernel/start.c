#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"
#include "stdio.h"

#include "test/affichage-test.c"
#include "test/processus-test1.c"
#include "test/processus-test2.c"
#include "test/horloge-test.c"
#include "processus.h"

void kernel_start(void)
{
	/* variables de tests */
	bool affichageT = 0;
	bool processusT1 = 0;
	bool processusT2 = 0;
	bool horlogeT = 0;

	/* initialisation */
	//call_debugger(); 				 // useless with qemu -s -S
	unsigned long quartz, ticks;
	clock_settings(&quartz, &ticks); // réglade de l'horloge
	efface_ecran();					 // efface l'écran

	/* tests */
	if (affichageT)
		affichageTest();
	if (processusT1)
		processusTest1();
	if (processusT2)
		processusTest2();
	if (horlogeT)
		horlogeTest();

	init_processus();
	idle();

	// boucle d'attente
	while (1)
		hlt();
	return;
}
