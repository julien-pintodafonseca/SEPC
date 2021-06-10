/* Test 2 des processus : idle_t2 passe la main à proc1_t2 et vice versa 3 fois */
#include "../processus.h"

struct processus file[NBPROC];

void idle_t2(void) // pid 0
{
    for (int i = 0; i < 3; i++)
    {
        printf("[idle] je tente de passer la main a proc1...\n");
        context_switch(0, 1);
        printf("[idle] proc1 m'a redonne la main\n");
    }
    printf("[idle] je bloque le systeme\n");
    hlt();
}

void proc1_t2(void) // pid 1
{
    for (;;)
    {
        printf("[proc1] idle m'a donne la main\n");
        printf("[proc1] je tente de lui la redonner...\n");
        context_switch(0, 1);
    }
}

void init_processus_t2(void)
{
    file[0].pid = 0;
    file[0].nom = "idle";
    file[0].etat = ACTIF;
    file[0].zone_sauv[1] = (int)(&file[0].pile[TAILLE_PILE - 1]);
    file[0].pile[TAILLE_PILE - 1] = (int)(idle_t2);

    file[1].pid = 1;
    file[1].nom = "proc1";
    file[1].etat = ACTIVABLE;
    file[1].zone_sauv[1] = (int)(&file[1].pile[TAILLE_PILE - 1]);
    file[1].pile[TAILLE_PILE - 1] = (int)(proc1_t2);
}

void processusTest2(void)
{
    // initialisation des structures de processus
    init_processus_t2();
    // demarrage du processus par defaut
    idle_t2();
}