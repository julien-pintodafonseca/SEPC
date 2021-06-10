#include "stdio.h"
#include "console.h"
#include "affichage.h"

void affichageTest(void)
{
    efface_ecran();

    console_putbytes("XXXXXXXXXXXXXXXXXXXXXXXX", 24);
    defilement();

    console_putbytes("  ov\bwo\ncoucou\rdebut\n\t\tcoucou\tcoucou", 36);

    printf("\n\nc\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\td");

    place_curseur(0, 0);
    ecrit_car(0, 0, 'A');

    place_curseur(24, 79);
    ecrit_car(24, 79, 'B');
}
