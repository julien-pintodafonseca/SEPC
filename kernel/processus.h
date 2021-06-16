#ifndef PROCESSUS_H_
#define PROCESSUS_H_

#include "stdint.h"

#define TAILLE_NOM 20
#define TAILLE_SAUV 5
#define TAILLE_PILE 512
#define NBPROC 30
#define MAXPRIO 256

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
    int parent;
    int fils[NBPROC];
    int retval;
};

struct processus procs[NBPROC];
struct processus *file_procs[NBPROC];
int proc_actif;

extern void ctx_sw(int *, int *);

/* Phase 2 : Traitement de l'horloge et des processus */
#include "stdio.h"
#include "stdbool.h"
#include "cpu.h"

#include "processus.h"
#include "horloge.h"

void context_switch(int old, int new);
int pidlibre(void);
void ordonnance(void);
void exit_proc_actif(void);
void exit_procs(int processus);
void exit(int retval);
int kill(int pid);
int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg);
int waitpid(int pid, int *retvalp);
int getproc(int pid);
int getprio(int pid);
int chprio(int pid, int newprio);
int getpid(void);

void proc1(void);
void proc2(void);
void proc3(void);
void proc4(void);
void idle(void);

#endif /* PROCESSUS_H_ */
