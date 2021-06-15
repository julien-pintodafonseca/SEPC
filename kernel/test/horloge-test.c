#include "stdio.h"

#include "start.h"
#include "horloge.h"

void horlogeTest(void)
{
    is_timer_printed = 1;
    masque_IRQ(0, 0);                     // démasquage de l'IRQ 0
    init_traitant_IT(32, traitant_IT_32); // initialisation du traitant 32
}
