/*******************************************************************************
 * Division 64 bits
 ******************************************************************************/
unsigned long long
div64(unsigned long long x, unsigned long long div, unsigned long long *rem)
{
	unsigned long long mul = 1;
	unsigned long long q;

	if ((div > x) || !div)
	{
		if (rem)
			*rem = x;
		return 0;
	}

	while (!((div >> 32) & 0x80000000ULL))
	{
		unsigned long long newd = div + div;
		if (newd > x)
			break;
		div = newd;
		mul += mul;
	}

	q = mul;
	x -= div;
	while (1)
	{
		mul /= 2;
		div /= 2;
		if (!mul)
		{
			if (rem)
				*rem = x;
			return q;
		}
		if (x < div)
			continue;
		q += mul;
		x -= div;
	}
}
/*******************************************************************************
 * Test 8
 *
 * Creation de processus se suicidant en boucle. Test de la vitesse de creation
 * de processus.
 ******************************************************************************/
static int
suicide(void *arg)
{
	(void)arg;
	kill(getpid());
	assert(0);
	return 0;
}

static int
suicide_launcher(void *arg)
{
	int pid1;
	(void)arg;
	printf("B");
	pid1 = start(suicide, 4000, 192, "suicide_launcher", 0);
	assert(pid1 > 0);
	printf("C");
	return pid1;
}

static void
test8(void)
{
	unsigned long long tsc1;
	unsigned long long tsc2;
	int i, r, pid, count;

	assert(getprio(getpid()) == 128);

	/* Le petit-fils va passer zombie avant le fils mais ne pas
	etre attendu par waitpid. Un nettoyage automatique doit etre
	fait. */
	pid = start(suicide_launcher, 4000, 129, "suicide_launcher", 0);
	assert(pid > 0);
	assert(waitpid(pid, &r) == pid);
	assert(chprio(r, 192) < 0);

	count = 0;
	__asm__ __volatile__("rdtsc"
						 : "=A"(tsc1));
	do
	{
		for (i = 0; i < 10; i++)
		{
			printf("A");
			pid = start(suicide_launcher, 4000, 200, "suicide_launcher", 0);
			printf("D");
			assert(pid > 0);
			assert(waitpid(pid, 0) == pid);
		}
		test_it();
		count += i;
		__asm__ __volatile__("rdtsc"
							 : "=A"(tsc2));
	} while ((tsc2 - tsc1) < 1000000000);
	printf("%lu cycles/process.\n", (unsigned long)div64(tsc2 - tsc1, 2 * count, 0));
}