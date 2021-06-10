/* Phase 2 : Traitement de l'horloge et des processus */
#include "stdio.h"
#include "cpu.h"
#include "processus.h"

struct processus file[NBPROC];

void context_switch(int old, int new)
{
    file[old].etat = ACTIVABLE;
    file[new].etat = ACTIF;
    ctx_sw(file[old].zone_sauv, file[new].zone_sauv);
}

/* Primitives de processus */
/*
int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg)
{
    // TODO
}

void exit(int retval)
{
}

int kill(int pid)
{
}

int waitpid(int pid, int *retvalp)
{
}

int getprio(int pid)
{
    if (pid < 0 || NBPROC <= pid)
    {
        return -1; // PID invalide
    }
    return file[getproc(pid)].prio;
}

int chprio(int pid, int newprio)
{
    if (pid < 0 || NBPROC <= pid)
    {
        return -1; // PID invalide
    }
    if (newprio <= 0 || MAXPRIO < newprio)
    {
        return -2; // newprio invalide
    }
    oldprio = getprio(pid);
    liste_proc[getproc(pid)].prio = newprio;
    return oldprio;
}

int getpid(void)
{
    return file[0].pid;
}

// retourne l'indice i correspondant au processeur de pid donné
int getproc(int pid)
{
    int i;
    for (i = 0; i < NBPROC && file[i].pid != pid; i++)
        ;
    return i;
}*/