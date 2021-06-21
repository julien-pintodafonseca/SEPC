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
        if (print_timer)
            print_time(str);
    }
    if (clk % (CLOCKFREQ / SCHEDFREQ) == 0)
    {
        /* vérification processus endormi */
        check_if_need_wake_up();
        /* vérification processus attendant un fils */
        check_if_child_is_end();
        /* changement de processus actif */
        ordonnance();
    }
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
    sleeping_file_procs[i].pid_wait = getpid();
    sleeping_file_procs[i].clk_wait = clock;
    ordonnance();
}

void init_sleeping_file_procs()
{
    for (int i = 0; i < NBPROC; i++)
    {
        sleeping_file_procs[i].pid_wait = -1;
        sleeping_file_procs[i].clk_wait = -1;
    }
}

void init_bloque_fils_file_procs()
{
    for (int i = 0; i < NBPROC; i++)
    {
        bloque_fils_file_procs[i].pid_pere = -1;
        bloque_fils_file_procs[i].pid_fils = -1;
    }
}

void check_if_need_wake_up()
{
    for (int i = 0; i < NBPROC; i++)
    {
        if (clk >= sleeping_file_procs[i].clk_wait)
        {
            file_procs[getproc(sleeping_file_procs[i].pid_wait)]->etat = ACTIVABLE;
            sleeping_file_procs[i].pid_wait = -1;
            sleeping_file_procs[i].clk_wait = -1;
        }
    }
}

void check_if_child_is_end()
{
    for (int i = 0; i < NBPROC; i++)
    {
        if (bloque_fils_file_procs[i].pid_pere != -1)
        {
            int proc_pere = getproc(bloque_fils_file_procs[i].pid_pere);
            if (bloque_fils_file_procs[i].pid_fils >= 0)
            {
                if (file_procs[getproc(bloque_fils_file_procs[i].pid_fils)]->etat == ZOMBIE)
                {
                    // le fils est fini
                    file_procs[proc_pere]->etat = ACTIVABLE;
                    bloque_fils_file_procs[i].pid_pere = -1;
                    bloque_fils_file_procs[i].pid_fils = -1;
                }
            }
            else
            {
                for (int j = 0; j < NBPROC; j++)
                {
                    if (file_procs[proc_pere]->fils[j] != -1 && file_procs[getproc(file_procs[proc_pere]->fils[j])]->etat == ZOMBIE)
                    {
                        // il existe un fils qui est déjà fini
                        file_procs[proc_pere]->etat = ACTIVABLE;
                        bloque_fils_file_procs[i].pid_pere = -1;
                        bloque_fils_file_procs[i].pid_fils = -1;
                        break;
                    }
                }
            }
        }
    }
}

/* Attend pendant le nombre de secondes waitsec */
void sleep(int waitsec)
{
    unsigned long cur = sec;
    sti();
    while (sec < cur + waitsec)
        ;
    cli();
}