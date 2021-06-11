/* Test 1 des processus : idle_t1 passe la main à proc1_t1 */
#include "processus.h"

struct processus file[NBPROC];

void idle_t1(void) // pid 0
{
    printf("[idle] je tente de passer la main a proc1...\n");
    context_switch(0, 1);
}

void proc1_t1(void) // pid 1
{
    printf("[proc1] idle m'a donne la main\n");
    printf("[proc1] j'arrete le systeme\n");
    hlt();
}

void init_processus_t1(void)
{
    file[0].pid = 0;
    sprintf(file[0].nom, "%p", "idle");
    file[0].etat = ACTIF;

    file[1].pid = 1;
    sprintf(file[1].nom, "%p", "proc1");
    file[1].etat = ACTIVABLE;
    file[1].zone_sauv[1] = (int)(&file[1].pile[TAILLE_PILE - 1]);
    file[1].pile[TAILLE_PILE - 1] = (int)(proc1_t1);
}

void processusTest1(void)
{
    // initialisation des structures de processus
    init_processus_t1();
    // demarrage du processus par defaut
    idle_t1();
}
