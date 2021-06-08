#include "../affichage.h"

void affichageTest(void)
{
    efface_ecran();

    console_putbytes("XXXXXXXXXXXXXXXXXXXXXXXX", 24);
    defilement();

    place_curseur(0, 0);
    ecrit_car(0, 0, 'A');

    place_curseur(24, 79);
    ecrit_car(24, 79, 'B');
}
