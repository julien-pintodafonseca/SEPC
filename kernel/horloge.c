#include "stdint.h"
#include "segment.h"
#include "cpu.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"

#include "affichage.h"
#include "horloge.h"
#include "processus.h"

uint32_t *idt = (uint32_t *)(0x1000); // table des vecteurs d'interruption (longueur 256)
unsigned long clk = 0;                // nombre d'interruptions
unsigned long sec = 0;                // nombre de secondes

struct sleeping_procs sleeping_file_procs[NBPROC]; // liste des processus endormis

void init_traitant_IT(int32_t num_IT, void (*traitant)(void))
{
    uint32_t w1 = (KERNEL_CS << 16) + ((uint32_t)traitant & 0x0000FFFF);
    uint32_t w2 = ((uint32_t)traitant & 0xFFFF0000) + (0x8E00);
    uint32_t *adr = (uint32_t *)(&idt + num_IT * 2);
    *adr = w1;
    *(adr + 1) = w2;
}

void clock_settings(unsigned long *quartz, unsigned long *ticks)
{
    *quartz = 0x1234DD;             // fréquence d'oscillation du quartz
    *ticks = (*quartz / CLOCKFREQ); // nombre d'oscillations du quartz entre chaque interruption
    outb(0x34, 0x43);
    outb(*ticks % 256, 0x40);
    outb(*ticks >> 8, 0x40);

    init_sleeping_file_procs(); // initialise la liste des processus endormis
}

void tic_PIT(void)
{
    outb(0x20, 0x20); // acquittement de l'interruption
    clk++;            // on compte l'interruption

    /* mise à jour de l'horloge */
    if (clk % CLOCKFREQ == 0)
    {
        sec++; // on compte une seconde
        char str[256];
        unsigned long hours = (sec / 60) / 60;
        unsigned long minutes = (sec / 60) % 60;
        unsigned long secondes = sec % 60;
        sprintf(str, "%.2ld:%.2ld:%.2ld", hours, minutes, secondes);
        print_time(str);
    }

    /* vérification processus endormi */
    check_if_need_wake_up();
}

void print_time(const char *chaine)
{
    uint32_t x = NB_COL - strlen(chaine);
    uint32_t y = 0;
    for (int i = 0; i < (int)(strlen(chaine)); i++)
    {
        ecrit_car(y, x, chaine[i]);
        x++;
    }
}

void masque_IRQ(uint32_t num_IRQ, bool masque)
{
    uint8_t actual_mask = inb(0x21);
    actual_mask ^= (-masque ^ actual_mask) & (1UL << num_IRQ); // actual_mask[num_IRQ] = masque;
    outb(actual_mask, 0x21);
}

unsigned long current_clock()
{
    return clk;
}

void wait_clock(unsigned long clock)
{
    asleep_proc(proc_actif, clock);
}

void init_sleeping_file_procs()
{
    for (int i = 0; i < NBPROC; i++)
    {
        sleeping_file_procs[i].pid_wait = -1;
        sleeping_file_procs[i].clk_wait = -1;
    }
}

void asleep_proc(int pid, unsigned long clock)
{
    if (file_procs[proc_actif]->etat == ACTIF)
        file_procs[proc_actif]->etat = ENDORMI;
    else
        return; // err

    int i = 0;
    while (sleeping_file_procs[i].pid_wait != -1)
    {
        if (i >= NBPROC)
            return; // err
        i++;
    }
    sleeping_file_procs[i].pid_wait = pid;
    sleeping_file_procs[i].clk_wait = clock;
}

void check_if_need_wake_up()
{
    for (int i = 0; i < NBPROC; i++)
    {
        if (clk >= sleeping_file_procs[i].clk_wait)
        {
            awake_proc(sleeping_file_procs[i].pid_wait);
        }
    }
}

void awake_proc(int pid)
{
    int i = 0;
    while (sleeping_file_procs[i].pid_wait != pid)
    {
        if (i >= NBPROC)
            return; // err
        i++;
    }
    file_procs[getproc(pid)]->etat = ACTIVABLE;
    sleeping_file_procs[i].pid_wait = -1;
    sleeping_file_procs[i].clk_wait = -1;
}

/* Attend pendant le nombre de secondes waitsec */
void sleep(int waitsec)
{
    unsigned long cur = sec;
    while (sec < cur + waitsec)
        ;
}