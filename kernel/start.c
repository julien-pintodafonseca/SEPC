#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"
#include "stdio.h"

#include "test/affichage-test.c"
#include "test/processus-test1.c"
#include "test/processus-test2.c"
#define DUMMY_VAL 78
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

	/*******************************************************************************
	 * Test 1
	 *
	 * Demarrage de processus avec passage de parametre
	 * Terminaison normale avec valeur de retour
	 * Attente de terminaison (cas fils avant pere et cas pere avant fils)
	 ******************************************************************************/

	int dummy1(void *arg)
	{
		printf("1");
		assert((int)arg == DUMMY_VAL);
		return 3;
	}

	int dummy1_2(void *arg)
	{
		printf(" 5");
		assert((int)arg == DUMMY_VAL + 1);
		return 4;
	}

	void test1(void)
	{
		int pid1;
		int r;
		int rval;

		pid1 = start(dummy1, 4000, 192, "paramRetour", (void *)DUMMY_VAL);
		assert(pid1 > 0);
		printf(" 2");
		r = waitpid(pid1, &rval);
		assert(r == pid1);
		assert(rval == 3);
		printf(" 3");
		pid1 = start(dummy1_2, 4000, 100, "paramRetour", (void *)(DUMMY_VAL + 1));
		assert(pid1 > 0);
		printf(" 4");
		r = waitpid(pid1, &rval);
		assert(r == pid1);
		assert(rval == 4);
		printf(" 6.\n");
	}

	void idle(void)
	{

		if (start((int (*)(void *))(test1), TAILLE_PILE - 1, 130, "test1", NULL) == -1)
			printf("erreur start test1\n");

		for (;;)
		{
			printf("[idle] pid = %i\n", getpid());
			sti();
			for (int i = 0; i < 100000000; i++)
				;
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
	procs[0].zone_sauv[1] = (int)(&procs[0].pile[TAILLE_PILE - 1]);
	procs[0].pile[TAILLE_PILE - 1] = (int)(idle);
	procs[0].parent = -1;
	for (int n = 0; n < NBPROC; n++)
	{
		procs[0].fils[n] = -1;
	}
	file_procs[0] = &procs[0];

	idle();

	// boucle d'attente
	while (1)
	{
		hlt();
	}
	return;
}
