/* Phase 2 : Traitement de l'horloge et des processus */
#include "stdio.h"
#include "stdbool.h"
#include "cpu.h"

#include "processus.h"
#include "horloge.h"

void context_switch(int old, int new)
{
    if (file_procs[old]->etat == ACTIF)
        file_procs[old]->etat = ACTIVABLE;
    file_procs[new]->etat = ACTIF;
    ctx_sw(file_procs[old]->zone_sauv, file_procs[new]->zone_sauv);
}

int pidlibre(void)
{
    int pid;
    bool exist = true;
    for (pid = -1; pid < NBPROC && exist; pid++)
    {
        exist = false;
        for (int i = -1; i < NBPROC; i++)
        {
            if (procs[i].pid == pid && procs[i].etat != ZOMBIE)
                exist = true;
        }
    }
    pid--;
    if (pid >= NBPROC)
        return -3;
    return pid;
}

void ordonnance(void)
{
    int old;
    old = proc_actif;
    do
    {
        proc_actif++;
        if (proc_actif >= NBPROC || file_procs[proc_actif]->prio < file_procs[old]->prio)
        {
            proc_actif = 0;
        }
    } while (file_procs[proc_actif]->etat != ACTIVABLE);
    context_switch(old, proc_actif);
}

void exit_procs(void)
{
    file_procs[proc_actif]->etat = ZOMBIE;
    ordonnance();
}

/* Primitives de processus */
int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg)
{
    int i;
    struct processus *tmp;
    for (i = 0; i < NBPROC && file_procs[i] != NULL; i++)
        ;
    if (i >= NBPROC)
        return -1;
    // On place le processus à la fin de la file
    int pid = pidlibre();
    procs[pid].pid = pid;
    sprintf(procs[i].nom, "%s", name);
    procs[pid].etat = ACTIVABLE;
    procs[pid].prio = prio;
    procs[pid].zone_sauv[1] = (int)(&procs[pid].pile[ssize - 2]);
    procs[pid].pile[ssize - 2] = (int)(pt_func);
    procs[pid].pile[ssize - 1] = (int)(exit_procs);
    procs[pid].pile[ssize] = (int)(arg);
    file_procs[i] = &procs[pid];
    // On remonte le processus tant que sa priorité
    // est supérieure au processus précédent dans la file
    int j = i;
    for (; i >= 1; i--)
    {
        if (file_procs[i - 1]->prio < file_procs[i]->prio)
        {
            tmp = file_procs[i - 1];
            file_procs[i - 1] = file_procs[i];
            file_procs[i] = tmp;
            j = i - 1;
        }
    }
    return file_procs[j]->pid;
}

void exit(int retval)
{
    // TODO
    file_procs[proc_actif]->etat = ZOMBIE;
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
    for (i = 0; i < NBPROC && file_procs[i]->pid != pid; i++)
        ;
    return i;
}

int getprio(int pid) // TODO VERIFIER
{
    if (pid < 0 || NBPROC <= pid)
    {
        return -1; // PID invalide
    }
    return file_procs[getproc(pid)]->prio;
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
    file_procs[getproc(pid)]->prio = newprio;
    return oldprio;
}

int getpid(void)
{
    return file_procs[proc_actif]->pid;
}

// TEST

void idle(void)
{
    for (;;)
    {
        printf("[idle] pid = %i\n", getpid());
        for (int i = 0; i < 100000000; i++)
            ;
        wait_clock(current_clock() + 5 * CLOCKFREQ);
        ordonnance();
    }
}

void proc1(void)
{
    for (;;)
    {
        printf("[proc1] pid = %i\n", getpid());
        for (int i = 0; i < 100000000; i++)
            ;
        wait_clock(current_clock() + 20 * CLOCKFREQ);
        ordonnance();
    }
}

void proc2(void)
{
    for (;;)
    {
        printf("[proc2] pid = %i\n", getpid());
        for (int i = 0; i < 100000000; i++)
            ;
        wait_clock(current_clock() + 10 * CLOCKFREQ);
        ordonnance();
    }
}

void proc3(void)
{
    for (;;)
    {
        printf("[proc3] pid = %i\n", getpid());
        for (int i = 0; i < 100000000; i++)
            ;
        wait_clock(current_clock() + 15 * CLOCKFREQ);
        ordonnance();
    }
}

void proc4(void)
{
    for (int i = 0; i < 2; i++)
    {
        printf("%d -> [proc4] pid = %i\n", i, getpid());
        sleep(2);
    }
}

void init_processus(void)
{
    for (int i = 0; i < NBPROC; i++)
    {
        procs[i].pid = -1;
    }
    procs[0].pid = 0;
    sprintf(procs[0].nom, "%p", "idle");
    procs[0].etat = ACTIF;
    procs[0].prio = MAXPRIO;
    procs[0].zone_sauv[1] = (int)(&procs[0].pile[TAILLE_PILE - 1]);
    procs[0].pile[TAILLE_PILE - 1] = (int)(idle);
    file_procs[0] = &procs[0];
    if (start((int (*)(void *))(proc1), TAILLE_PILE - 1, MAXPRIO, "proc1", NULL) == -1)
        printf("erreur start proc1\n");
    if (start((int (*)(void *))(proc2), TAILLE_PILE - 1, MAXPRIO, "proc2", NULL) == -1)
        printf("erreur start proc2\n");
    if (start((int (*)(void *))(proc3), TAILLE_PILE - 1, MAXPRIO, "proc3", NULL) == -1)
        printf("erreur start proc3\n");
    if (start((int (*)(void *))(proc4), TAILLE_PILE - 1, MAXPRIO, "proc4", NULL) == -1)
        printf("erreur start proc4\n");
}