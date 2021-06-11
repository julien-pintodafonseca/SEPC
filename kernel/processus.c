/* Phase 2 : Traitement de l'horloge et des processus */
#include "stdio.h"
#include "cpu.h"
#include "processus.h"

struct processus file[NBPROC];
int proc_actif;

void context_switch(int old, int new)
{
    file[old].etat = ACTIVABLE;
    file[new].etat = ACTIF;
    ctx_sw(file[old].zone_sauv, file[new].zone_sauv);
}

/* Primitives de processus */
int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg)
{
    int i;
    for (i = 0; i < NBPROC && file[i].pid == i; i++)
        ;
    file[i].pid = i; // TODO redef pid
    sprintf(file[i].nom, "%p", name);
    file[i].etat = ACTIVABLE;
    file[i].prio = prio;
    file[i].zone_sauv[1] = (int)(&file[2].pile[ssize]);
    file[i].pile[ssize] = (int)(pt_func);
    return file[i].pid;
}

void exit(int retval)
{
    // TODO
    printf("%d", retval);
    while (1)
        ;
}

int kill(int pid)
{
    // TODO
    printf("%d", pid);
    return 0;
}

int waitpid(int pid, int *retvalp)
{
    // TODO
    printf("%d %d", pid, *retvalp);
    return 0;
}

/* Retourne l'indice i correspondant au processeur de pid donné */
int getproc(int pid)
{
    int i;
    for (i = 0; i < NBPROC && file[i].pid != pid; i++)
        ;
    return i;
}

int getprio(int pid) // TODO VERIFIER
{
    if (pid < 0 || NBPROC <= pid)
    {
        return -1; // PID invalide
    }
    return file[getproc(pid)].prio;
}

int chprio(int pid, int newprio) // TODO VERIFIER
{
    if (pid < 0 || NBPROC <= pid)
    {
        return -1; // PID invalide
    }
    if (newprio <= 0 || MAXPRIO < newprio)
    {
        return -2; // newprio invalide
    }
    int oldprio = getprio(pid);
    file[getproc(pid)].prio = newprio;
    return oldprio;
}

int getpid(void)
{
    return file[proc_actif].pid;
}

// TEST

void ordonnance(void)
{
    int old, new;
    old = proc_actif;
    new = proc_actif + 1;
    if (new >= NBPROC)
    {
        new = 0;
    }
    proc_actif = new;
    context_switch(old, new);
}

void idle(void)
{
    for (;;)
    {
        printf("[idle] pid = %i\n", getpid());
        for (int i = 0; i < 100000000; i++)
            ;
        ordonnance();
    }
}

void proc1(void)
{
    for (;;)
    {
        printf("[processus 1] pid = %i\n", getpid());
        for (int i = 0; i < 100000000; i++)
            ;
        ordonnance();
    }
}

void proc2(void)
{
    for (;;)
    {
        printf("[processus 2] pid = %i\n", getpid());
        for (int i = 0; i < 100000000; i++)
            ;
        ordonnance();
    }
}

void proc3(void)
{
    for (;;)
    {
        printf("[processus 3] pid = %i\n", getpid());
        for (int i = 0; i < 100000000; i++)
            ;
        ordonnance();
    }
}

void init_processus(void)
{
    file[0].pid = 0;
    sprintf(file[0].nom, "%p", "idle");
    file[0].etat = ACTIF;
    file[0].prio = 0;
    file[0].zone_sauv[1] = (int)(&file[0].pile[TAILLE_PILE - 1]);
    file[0].pile[TAILLE_PILE - 1] = (int)(idle);

    start((int (*)(void *))(proc1), TAILLE_PILE - 1, 0, "proc1", NULL);
    start((int (*)(void *))(proc2), TAILLE_PILE - 1, 0, "proc2", NULL);
    start((int (*)(void *))(proc3), TAILLE_PILE - 1, 0, "proc3", NULL);
}