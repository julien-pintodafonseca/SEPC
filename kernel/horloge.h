#ifndef HORLOGE_H_
#define HORLOGE_H_

#include "stdint.h"
#include "processus.h"

#define CLOCKFREQ 300 // fréquence d'interruption (entre 100Hz et 1000Hz)
#define SCHEDFREQ 50  // fréquence de préemption des processus (enter 50Hz et 1000Hz)

struct sleeping_procs
{
    int pid_wait;           // pid
    unsigned long clk_wait; // heure de réveille
};

struct bloque_fils_procs
{
    int pid_pere; // pid du père bloqué
    int pid_fils; // pid du fils attendu (négatif = n'importe quel fils)
};

struct sleeping_procs sleeping_file_procs[NBPROC];       // liste des processus endormis
struct bloque_fils_procs bloque_fils_file_procs[NBPROC]; // liste des processus attendant un fils

extern void traitant_IT_32(void);

void init_traitant_IT(int32_t num_IT, void (*traitant)(void));
void clock_settings(unsigned long *quartz, unsigned long *ticks);
void tic_PIT(void);
void print_time(const char *chaine);
void masque_IRQ(uint32_t num_IRQ, bool masque);
void sleep(int waitsec);
unsigned long current_clock();
void wait_clock(unsigned long clock);
void init_sleeping_file_procs();
void init_bloque_fils_file_procs();
void check_if_need_wake_up();
void check_if_child_is_end();
void awake_proc(int pid);

#endif /* HORLOGE_H_ */
