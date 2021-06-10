#include "stdio.h"
#include "horloge.h"

void horlogeTest(void)
{
    printf("start\n");
    masque_IRQ(0, 0);                     // démasquage de l'IRQ 0
    init_traitant_IT(32, traitant_IT_32); // initialisation du traitant 32
    sti();                                // démasquage des interruptions externes
}
