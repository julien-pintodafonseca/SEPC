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
const int loop_count0 = 5000;
const int loop_count1 = 10000;

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
	printf(" - 2"); // il s'arrête ici /!\ A B A B B A A ??
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
