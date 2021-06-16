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
	 * Test 5
	 *
	 * Tests de quelques parametres invalides.
	 * Certaines interdictions ne sont peut-etre pas dans la spec. Changez les pour
	 * faire passer le test correctement.
	 ******************************************************************************/

	int no_run(void *arg)
	{
		(void)arg;
		assert(0);
		return 1;
	}

	int waiter(void *arg)
	{
		int pid = (int)arg;
		assert(kill(pid) == 0);
		assert(waitpid(pid, 0) < 0);
		return 1;
	}

	void test5(void)
	{
		int pid1, pid2;
		int r;

		// Le processus 0 et la priorite 0 sont des parametres invalides
		assert(kill(0) < 0);
		assert(chprio(getpid(), 0) < 0);
		assert(getprio(getpid()) == 128);
		pid1 = start(no_run, 4000, 64, "norun", 0);
		assert(pid1 > 0);
		assert(kill(pid1) == 0);
		assert(kill(pid1) < 0);		   //pas de kill de zombie
		assert(chprio(pid1, 128) < 0); //changer la priorite d'un zombie
		assert(chprio(pid1, 64) < 0);  //changer la priorite d'un zombie
		assert(waitpid(pid1, 0) == pid1);
		assert(waitpid(pid1, 0) < 0);
		pid1 = start(no_run, 4000, 64, "norun", 0);
		assert(pid1 > 0);
		pid2 = start(waiter, 4000, 65, "waiter", (void *)pid1);
		assert(pid2 > 0);
		assert(waitpid(pid2, &r) == pid2);
		assert(r == 1);
		assert(waitpid(pid1, &r) == pid1);
		assert(r == 0);
		printf("ok.\n");
	}

	void idle(void)
	{
		if (start((int (*)(void *))(test5), TAILLE_PILE - 1, 128, "test5", NULL) == -1)
			printf("erreur start test5\n");

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
