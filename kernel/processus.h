#ifndef PROCESSUS_H_
#define PROCESSUS_H_

#include "stdint.h"
#include "queue.h"
#include "stdio.h"
#include "stdbool.h"
#include "cpu.h"

#define TAILLE_NOM 20
#define TAILLE_SAUV 5
#define HEAP_LEN 15 << 20
#define NBPROC 30
#define MAXPRIO 256

#include "processus.h"
#include "horloge.h"

typedef enum ETAT
{
    ACTIF, // ELU
    ACTIVABLE,
    BLOQUE_FMSG_PLEINE, // Attente d'un message en file pleine
    BLOQUE_FMSG_VIDE,   // Attente d'un message en file vide
    BLOQUE_SEMAPHORE,
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
    int parent;
    int fils[NBPROC];
    int retval;

    // gestion pile
    int *pile;
    int taille_pile;

    // file de messages
    int lastmsg;
    bool del;
};

struct processus procs[NBPROC];
struct processus *file_procs[NBPROC];
int proc_actif, pid_sauv;

extern void ctx_sw(int *, int *);

void initialize(int (*pt_func)(void *), const char *name);
int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg);
int pidlibre(void);
int getpid(void);
int getproc(int pid);
int getprio(int pid);
void ordonnance(void);
void context_switch(int old, int new);
void exit_proc_actif(void);
void exit(int retval);
void exit_procs(int processus);
int kill(int pid);
int waitpid(int pid, int *retvalp);
int chprio(int pid, int newprio);
void print_procs(void);

#endif /* PROCESSUS_H_ */
