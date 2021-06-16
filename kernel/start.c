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
	 * Test 3
	 *
	 * chprio() et ordre de scheduling
	 * kill() d'un processus qui devient moins prioritaire
	 ******************************************************************************/
	int proc_prio4(void *arg)
	{
		/* arg = priority of this proc. */
		int r;

		assert(getprio(getpid()) == (int)arg);
		printf("1");
		r = chprio(getpid(), 64);
		assert(r == (int)arg);
		printf(" 3");
		return 0;
	}

	int proc_prio5(void *arg)
	{
		/* Arg = priority of this proc. */
		int r;

		assert(getprio(getpid()) == (int)arg);
		printf(" 7");
		r = chprio(getpid(), 64);
		assert(r == (int)arg);
		printf("error: I should have been killed\n");
		assert(0);
		return 0;
	}

	void test3(void)
	{
		int pid1;
		int p = 192;
		int r;

		assert(getprio(getpid()) == 128);
		pid1 = start(proc_prio4, 4000, p, "prio", (void *)p);
		assert(pid1 > 0);
		printf(" 2");
		r = chprio(getpid(), 32);
		assert(r == 128);
		printf(" 4");
		r = chprio(getpid(), 128);
		assert(r == 32);
		printf(" 5");
		assert(waitpid(pid1, 0) == pid1);
		printf(" 6");

		assert(getprio(getpid()) == 128);
		pid1 = start(proc_prio5, 4000, p, "prio", (void *)p);
		assert(pid1 > 0);
		printf(" 8");
		r = kill(pid1);
		assert(r == 0);
		assert(waitpid(pid1, 0) == pid1);
		printf(" 9");
		r = chprio(getpid(), 32);
		assert(r == 128);
		printf(" 10");
		r = chprio(getpid(), 128);
		assert(r == 32);
		printf(" 11.\n");
	}

	void idle(void)
	{
		if (start((int (*)(void *))(test3), TAILLE_PILE - 1, 128, "test3", NULL) == -1)
			printf("erreur start test3\n");

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
