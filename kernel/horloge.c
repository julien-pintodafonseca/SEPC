#include "stdint.h"
#include "segment.h"
#include "cpu.h"
#include "string.h"
#include "stdbool.h"

#include "affichage.h"
#include "printf.c"
#include "sprintf.c"

uint64_t *idt = (uint64_t *)(0x1000);   // table des vecteurs d'interruption (=> longueur 256)
const int CLOCKFREQ = 300;              // fréquence d'interruption, entre 100Hz et 1000Hz
unsigned long clock = 0;                // nombre d'interruptions

void init_traitant_IT(int32_t num_IT, void (*traitant)(void))
{
    uint32_t w1 = (KERNEL_CS << 16) | ((int)traitant & 0x0F);
    uint32_t w2 = ((int)traitant & 0xF0) | (0x8E00);
    uint64_t entry = (w1 << 31) | w2;
    idt[num_IT] = entry;
}

void show_time(const char *chaine)
{
    uint32_t x = 79 - strlen(chaine);
    uint32_t y = 0;
    place_curseur(x, y);
    printf("%s", chaine);
}

unsigned long current_clock()
{
    return clock;
}

void tic_PIT(void)
{
    outb(0x20, 0x20); // acquittement de l'interruption
    clock++;
    char time[256];
    sprintf(time, "%ld", current_clock());
    show_time(time);
}

void clock_settings(unsigned long *quartz, unsigned long *ticks)
{
    *quartz = 0x1234DD; // fréquence d'oscillation du quartz
    *ticks = (*quartz / CLOCKFREQ) % 256; // nombre d'oscillations du quartz entre chaque interruption
    outb(0x34, 0x43);
    outb(*ticks, 0x40);
    outb(*ticks >> 8, 0x40);
}

void masque_IRQ(uint32_t num_IRQ, bool masque)
{
    uint8_t actual_mask = inb(0x21);
    actual_mask ^= (-masque ^ actual_mask) & (1UL << num_IRQ);  //actual_mask[num_IRQ] = masque;
    outb(actual_mask, 0x21);
}

/*
void wait_clock(unsigned long clock)
{
    // TODO
}
*/
