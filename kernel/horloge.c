#include "stdint.h"
#include "segment.h"
#include "cpu.h"
#include "string.h"
#include "stdbool.h"
#include "stdio.h"

#include "affichage.h"
#include "horloge.h"

uint64_t *idt = (uint64_t *)(0x1000); // table des vecteurs d'interruption (=> longueur 256)
const int CLOCKFREQ = 50;             // fréquence d'interruption, entre 100Hz et 1000Hz
unsigned long clock = 0;              // nombre d'interruptions

void init_traitant_IT(int32_t num_IT, void (*traitant)(void))
{
    uint32_t w1 = (KERNEL_CS << 16) + ((uint32_t)traitant & 0x0000FFFF);
    uint32_t w2 = ((uint32_t)traitant & 0xFFFF0000) + (0x8E00);
    idt[num_IT] = ((uint64_t)(w2) << 32) | (uint64_t)(w1);
    printf("KERNEL_CS   %X\n", KERNEL_CS);
    printf("traitant    %X\n", (uint32_t)(traitant));
    printf("w1          %X\n", w1);
    printf("w2          %X\n", w2);
    printf("Res         %llX\n", ((uint64_t)(w2) << 32) | (uint64_t)(w1));
    printf("idt[num_IT] %llX", idt[num_IT]);
}

void clock_settings(unsigned long *quartz, unsigned long *ticks)
{
    *quartz = 0x1234DD;                   // fréquence d'oscillation du quartz
    *ticks = (*quartz / CLOCKFREQ) % 256; // nombre d'oscillations du quartz entre chaque interruption
    // ici ???
    outb(0x34, 0x43);
    outb(*ticks, 0x40);
    outb(*ticks >> 8, 0x40);
}

void tic_PIT(void)
{
    outb(0x20, 0x20); // acquittement de l'interruption
    clock++;          // Toutes les 20ms

    char str[256];
    unsigned long cur_clock = current_clock();
    unsigned long hours = 0;                               // TODO calculer correctement
    unsigned long minutes = 0;                             // TODO calculer correctement
    unsigned long secondes = cur_clock;                    // TODO calculer correctement
    sprintf(str, "%ld:%ld:%ld", hours, minutes, secondes); // TODO améliorer affichage
    show_time(str);
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
