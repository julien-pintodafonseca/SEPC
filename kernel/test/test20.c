

#define NR_PHILO 5

/*******************************************************************************
 * Test 20
 *
 * Le repas des philosophes.
 ******************************************************************************/
char f[NR_PHILO]; /* tableau des fourchettes, contient soit 1 soit 0 selon si elle
			    est utilisee ou non */

char bloque[NR_PHILO]; /* memorise l'etat du philosophe, contient 1 ou 0 selon que le philosophe
				 est en attente d'une fourchette ou non */

struct sem mutex_philo; /* exclusion mutuelle */
struct sem s[NR_PHILO]; /* un semaphore par philosophe */
int etat[NR_PHILO];

void affiche_etat()
{
    int i;
    printf("%c", 13);
    for (i = 0; i < NR_PHILO; i++)
    {
        unsigned long c;
        switch (etat[i])
        {
        case 'm':
            c = 2;
            break;
        default:
            c = 4;
        }
        int tmp = c % 2;
        assert(tmp == 0); // utilisation de c pour le compilo
        printf("%c", etat[i]);
    }
}

void waitloop(void)
{
    int j;
    for (j = 0; j < 5000; j++)
    {
        int l;
        test_it();
        for (l = 0; l < 5000; l++)
            ;
    }
}

void penser(long i)
{
    xwait(&mutex_philo); /* DEBUT SC */
    etat[i] = 'p';
    affiche_etat();
    xsignal(&mutex_philo); /* Fin SC */
    waitloop();
    xwait(&mutex_philo); /* DEBUT SC */
    etat[i] = '-';
    affiche_etat();
    xsignal(&mutex_philo); /* Fin SC */
}

void manger(long i)
{
    xwait(&mutex_philo); /* DEBUT SC */
    etat[i] = 'm';
    affiche_etat();
    xsignal(&mutex_philo); /* Fin SC */
    waitloop();
    xwait(&mutex_philo); /* DEBUT SC */
    etat[i] = '-';
    affiche_etat();
    xsignal(&mutex_philo); /* Fin SC */
}

int test(int i)
{
    /* les fourchettes du philosophe i sont elles libres ? */
    return ((!f[i] && (!f[(i + 1) % NR_PHILO])));
}

void prendre_fourchettes(int i)
{
    /* le philosophe i prend des fourchettes */

    xwait(&mutex_philo); /* Debut SC */

    if (test(i))
    { /* on tente de prendre 2 fourchette */
        f[i] = 1;
        f[(i + 1) % NR_PHILO] = 1;
        xsignal(&s[i]);
    }
    else
        bloque[i] = 1;

    xsignal(&mutex_philo); /* FIN SC */
    xwait(&s[i]);          /* on attend au cas o on ne puisse pas prendre 2 fourchettes */
}

void poser_fourchettes(int i)
{

    xwait(&mutex_philo); /* DEBUT SC */

    if ((bloque[(i + NR_PHILO - 1) % NR_PHILO]) && (!f[(i + NR_PHILO - 1) % NR_PHILO]))
    {
        f[(i + NR_PHILO - 1) % NR_PHILO] = 1;
        bloque[(i + NR_PHILO - 1) % NR_PHILO] = 0;
        xsignal(&s[(i + NR_PHILO - 1) % NR_PHILO]);
    }
    else
        f[i] = 0;

    if ((bloque[(i + 1) % NR_PHILO]) && (!f[(i + 2) % NR_PHILO]))
    {
        f[(i + 2) % NR_PHILO] = 1;
        bloque[(i + 1) % NR_PHILO] = 0;
        xsignal(&s[(i + 1) % NR_PHILO]);
    }
    else
        f[(i + 1) % NR_PHILO] = 0;

    xsignal(&mutex_philo); /* Fin SC */
}

int philosophe(void *arg)
{
    /* comportement d'un seul philosophe */
    int i = (int)arg;
    int k;

    for (k = 0; k < 6; k++)
    {
        prendre_fourchettes(i); /* prend 2 fourchettes ou se bloque */
        manger(i);              /* le philosophe mange */
        poser_fourchettes(i);   /* pose 2 fourchettes */
        penser(i);              /* le philosophe pense */
    }
    xwait(&mutex_philo); /* DEBUT SC */
    etat[i] = '-';
    affiche_etat();
    xsignal(&mutex_philo); /* Fin SC */
    return 0;
}

int launch_philo()
{

    int i, pid;

    for (i = 0; i < NR_PHILO; i++)
    {
        pid = start(philosophe, 4000, 192, "philosophe", (void *)i);
        assert(pid > 0);
    }
    return 0;
}

void test20(void)
{
    int j, pid;

    xscreate(&mutex_philo); /* semaphore d'exclusion mutuelle */
    xsignal(&mutex_philo);
    for (j = 0; j < NR_PHILO; j++)
    {
        xscreate(s + j); /* semaphore de bloquage des philosophes */
        f[j] = 0;
        bloque[j] = 0;
        etat[j] = '-';
    }

    printf("\n");
    pid = start(launch_philo, 4000, 193, "Lanceur philosophes", 0);
    assert(pid > 0);
    assert(waitpid(pid, 0) == pid);
    printf("\n");
    xsdelete(&mutex_philo);
    for (j = 0; j < NR_PHILO; j++)
    {
        xsdelete(s + j);
    }
}
