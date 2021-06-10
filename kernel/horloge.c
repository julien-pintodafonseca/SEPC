#include "stdint.h"
#include "segment.h"
#include "cpu.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"

#include "affichage.h"
#include "horloge.h"

uint32_t *idt = (uint32_t *)(0x1000); // table des vecteurs d'interruption (longueur 256)
const int CLOCKFREQ = 300;            // fréquence d'interruption (entre 100Hz et 1000Hz)
unsigned long clock = 0;              // nombre d'interruptions
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
    outb(0x34, 0x43);
    outb(*ticks % 256, 0x40);
    outb(*ticks >> 8, 0x40);
}

void tic_PIT(void)
{
    outb(0x20, 0x20); // acquittement de l'interruption
    clock++;          // on compte l'interruption
    if (clock % CLOCKFREQ == 0)
    {
        sec++; // on compte une seconde
        char str[256];
        unsigned long hours = (sec / 60) / 60;
        unsigned long minutes = (sec / 60) % 60;
        unsigned long secondes = sec % 60;
        sprintf(str, "%.2ld:%.2ld:%.2ld", hours, minutes, secondes);
        print_time(str);
    }
}

void print_time(const char *chaine)
{
    uint32_t x = 80 - strlen(chaine);
    uint32_t y = 0;
    place_curseur(y, x);
    printf("%s", chaine);
}

void wait_clock(unsigned long clock)
{
    // Passe le processus dans l'état endormi
    // attend le nombre d'interruptions en param atteint ou dépassé
    printf("wait_clock(%ld)", clock); // TODO plus tard
}

void masque_IRQ(uint32_t num_IRQ, bool masque)
{
    uint8_t actual_mask = inb(0x21);
    actual_mask ^= (-masque ^ actual_mask) & (1UL << num_IRQ); // actual_mask[num_IRQ] = masque;
    outb(actual_mask, 0x21);
}

unsigned long current_clock()
{
    return clock;
}
