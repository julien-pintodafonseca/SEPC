#ifndef PROCESSUS_H_
#define PROCESSUS_H_

#include "stdint.h"

#define TAILLE_NOM 20
#define TAILLE_SAUV 5
#define TAILLE_PILE 512
#define NBPROC 4
#define MAXPRIO 256
#define SCHEDFREQ 100 // Hz

extern void ctx_sw(int *, int *);

typedef enum ETAT
{
    ACTIF, // ELU
    ACTIVABLE,
    BLOQUE_SEMAPHORE,
    BLOQUE_ES,
    BLOQUE_FILS,
    ENDORMI,
    ZOMBIE
} ETAT;

struct processus
{
    int pid;
    char nom[TAILLE_NOM];
    ETAT etat;
    int prio;
    int zone_sauv[TAILLE_SAUV];
    int pile[TAILLE_PILE];
};

struct processus procs[NBPROC];
struct processus *file_procs[NBPROC];
int proc_actif;

void context_switch(int old, int new);
int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name); //, void *arg);
void exit(int retval);
int kill(int pid);
int waitpid(int pid, int *retvalp);
int getprio(int pid);
int chprio(int pid, int newprio);
int getpid(void);
int getproc(int pid);

void init_processus(void);
void idle(void);

#endif /* PROCESSUS_H_ */
