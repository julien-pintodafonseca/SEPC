#include "debugger.h"
#include "cpu.h"
#include "stdbool.h"
#include "stdio.h"

#include "test/affichage-test.c"
#include "test/processus-test1.c"
#include "test/processus-test2.c"
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
	 * Test 2
	 *
	 * kill() de fils suspendu pas demarre
	 * waitpid() de ce fils termine par kill()
	 * waitpid() de fils termine par exit()
	 ******************************************************************************/
	int dummy2(void *args)
	{
		printf(" X");
		return (int)args;
	}

	int dummy2_2(void *args)
	{
		printf(" 5");
		exit((int)args);
		assert(0);
		return 0;
	}

	void test2(void)
	{
		int rval;
		int r;
		int pid1;
		int val = 45;

		printf("1");
		pid1 = start(dummy2, 4000, 100, "procKill", (void *)val);
		assert(pid1 > 0);
		printf(" 2");
		r = kill(pid1);
		assert(r == 0);
		printf(" 3");
		r = waitpid(pid1, &rval);
		assert(rval == 0);
		assert(r == pid1);
		printf(" 4");
		pid1 = start(dummy2_2, 4000, 192, "procExit", (void *)val);
		assert(pid1 > 0);
		printf(" 6");
		r = waitpid(pid1, &rval);
		assert(rval == val);
		assert(r == pid1);
		assert(waitpid(getpid(), &rval) < 0);
		printf(" 7.\n");
	}

	void idle(void)
	{
		if (start((int (*)(void *))(test2), TAILLE_PILE - 1, 130, "test2", NULL) == -1)
			printf("erreur start test2\n");

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
