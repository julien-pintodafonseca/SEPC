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
	 * Test 6
	 *
	 * Waitpid multiple.
	 * Creation de processus avec differentes tailles de piles.
	 ******************************************************************************/
	extern int __proc6_1(void *arg);
	extern int __proc6_2(void *arg);

	void test6(void)
	{
		int pid1, pid2, pid3;
		int ret;

		assert(getprio(getpid()) == 128);
		pid1 = start(__proc6_1, 0, 64, "proc6_1", 0);
		assert(pid1 > 0);
		pid2 = start(__proc6_2, 4, 66, "proc6_2", (void *)4);
		assert(pid2 > 0);
		pid3 = start(__proc6_2, 0xffffffff, 65, "proc6_3", (void *)5);
		assert(pid3 < 0);
		pid3 = start(__proc6_2, 8, 65, "proc6_3", (void *)5);
		assert(pid3 > 0);
		assert(waitpid(-1, &ret) == pid2);
		assert(ret == 4);
		assert(waitpid(-1, &ret) == pid3);
		assert(ret == 5);
		assert(waitpid(-1, &ret) == pid1);
		assert(ret == 3);
		assert(waitpid(pid1, 0) < 0);
		assert(waitpid(-1, 0) < 0);
		assert(waitpid(getpid(), 0) < 0);
		printf("ok.\n");
	}

	void idle(void)
	{
		if (start((int (*)(void *))(test6), TAILLE_PILE - 1, 128, "test6", NULL) == -1)
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
	procs[0].zone_sauv[1] = (int)(&procs[0].pile[TAILLE_PILE - 1]);
	procs[0].pile[TAILLE_PILE - 1] = (int)(idle);
	procs[0].parent = -1;
	for (int n = 0; n < NBPROC; n++)
	{
		procs[0].fils[n] = -1;
	}
	file_procs[0] = &procs[0];

	idle();
	return;
}
