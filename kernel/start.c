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
	 * Unmask interrupts for those who are working in kernel mode
	 ******************************************************************************/
	void test_it()
	{
		__asm__ volatile("pushfl; testl $0x200,(%%esp); jnz 0f; sti; nop; cli; 0: addl $4,%%esp\n" ::
							 : "memory");
	}

	/*******************************************************************************
 * Test 4
 *
 * Boucles d'attente active (partage de temps)
 * chprio()
 * kill() de processus de faible prio
 * kill() de processus deja mort
 ******************************************************************************/
	static const int loop_count0 = 5000;
	static const int loop_count1 = 10000;

	int busy_loop1(void *arg)
	{
		(void)arg;
		while (1)
		{
			int i, j;

			printf(" A");
			for (i = 0; i < loop_count1; i++)
			{
				test_it();
				for (j = 0; j < loop_count0; j++)
					;
			}
		}
		return 0;
	}

	/* assume the process to suspend has a priority == 64 */
	int busy_loop2(void *arg)
	{
		int i;

		for (i = 0; i < 3; i++)
		{
			int k, j;

			printf(" B");
			for (k = 0; k < loop_count1; k++)
			{
				test_it();
				for (j = 0; j < loop_count0; j++)
					;
			}
		}
		i = chprio((int)arg, 16);
		assert(i == 64);
		return 0;
	}

	void test4(void)
	{
		int pid1, pid2;
		int r;
		int arg = 0;

		assert(getprio(getpid()) == 128);
		pid1 = start(busy_loop1, 4000, 64, "busy1", (void *)arg);
		assert(pid1 > 0);
		pid2 = start(busy_loop2, 4000, 64, "busy2", (void *)pid1);
		assert(pid2 > 0);
		printf("1 -");
		r = chprio(getpid(), 32);
		assert(r == 128);
		printf(" - 2");
		r = kill(pid1);
		assert(r == 0);
		assert(waitpid(pid1, 0) == pid1);
		r = kill(pid2);
		assert(r < 0); /* kill d'un processus zombie */
		assert(waitpid(pid2, 0) == pid2);
		printf(" 3");
		r = chprio(getpid(), 128);
		assert(r == 32);
		printf(" 4.\n");
	}

	void idle(void)
	{
		if (start((int (*)(void *))(test4), TAILLE_PILE - 1, 128, "test4", NULL) == -1)
			printf("erreur start test4\n");

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
