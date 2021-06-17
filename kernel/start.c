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
	 * Test 7
	 *
	 * Test de l'horloge (ARR et ACE)
	 * Tentative de determination de la frequence du processeur et de la
	 * periode de scheduling
	 ******************************************************************************/
	void test_it()
	{
		__asm__ volatile("pushfl; testl $0x200,(%%esp); jnz 0f; sti; nop; cli; 0: addl $4,%%esp\n" ::
							 : "memory");
	}

	int proc_timer1(void *arg)
	{
		unsigned long quartz;
		unsigned long ticks;
		unsigned long dur;
		int i;

		(void)arg;

		clock_settings(&quartz, &ticks);
		dur = (quartz + ticks) / ticks;
		printf(" 2");
		for (i = 4; i < 8; i++)
		{
			wait_clock(current_clock() + dur);
			printf(" %d", i);
		}
		return 0;
	}

	volatile unsigned long timer;

	int proc_timer(void *arg)
	{
		(void)arg;
		while (1)
		{
			unsigned long t = timer + 1;
			timer = t;
			while (timer == t)
				test_it();
		}
		while (1)
			;
		return 0;
	}

	int sleep_pr1(void *args)
	{
		(void)args;
		wait_clock(current_clock() + 2);
		printf(" not killed !!!");
		assert(0);
		return 1;
	}

	void test7(void)
	{
		int pid1, pid2, r;
		unsigned long c0, c, quartz, ticks, dur;

		assert(getprio(getpid()) == 128);
		printf("1");
		pid1 = start(proc_timer1, 4000, 129, "timer", 0);
		assert(pid1 > 0);
		printf(" 3");
		assert(waitpid(-1, 0) == pid1);
		printf(" 8 : ");

		timer = 0;
		pid1 = start(proc_timer, 4000, 127, "timer1", 0);
		pid2 = start(proc_timer, 4000, 127, "timer2", 0);
		assert(pid1 > 0);
		assert(pid2 > 0);
		clock_settings(&quartz, &ticks);
		dur = 2 * quartz / ticks;
		test_it();
		c0 = current_clock();
		do
		{
			test_it();
			c = current_clock();
		} while (c == c0);
		wait_clock(c + dur);
		assert(kill(pid1) == 0);
		assert(waitpid(pid1, 0) == pid1);
		assert(kill(pid2) == 0);
		assert(waitpid(pid2, 0) == pid2);
		printf("%lu changements de contexte sur %lu tops d'horloge", timer, dur);
		pid1 = start(sleep_pr1, 4000, 192, "sleep", 0);
		assert(pid1 > 0);
		assert(kill(pid1) == 0);
		assert(waitpid(pid1, &r) == pid1);
		assert(r == 0);
		printf(".\n");
	}

	void idle(void)
	{
		if (start((int (*)(void *))(test7), TAILLE_PILE - 1, 128, "test7", NULL) == -1)
			printf("erreur start test7\n");

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
