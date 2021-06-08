/* Phase 1 : Affichage à l'écran */
/*   Fonction console_putbytes   */

#include "stdint.h"
#include "cpu.h"
#include "string.h"

uint32_t x = 0; // ligne
uint32_t y = 0; // colonne

uint16_t *ptr_mem(uint32_t lig, uint32_t col)
{
    return (uint16_t *)(0xB8000 + 2 * (lig * 80 + col));
}

void ecrit_car(uint32_t lig, uint32_t col, char c)
{
    uint16_t *addr = ptr_mem(lig, col);
    uint8_t format = (0 << 7) | (0 << 4) | (15);
    *addr = (format << 8) | c;
}

void place_curseur(uint32_t lig, uint32_t col)
{
    uint16_t pos = col + lig * 80;
    outb(0x0F, 0x3D4);
    outb(pos, 0x3D5);
    outb(0x0E, 0x3D4);
    outb(pos >> 8, 0x3D5);
    x = lig;
    y = col;
}

void efface_ecran(void)
{
    for (int col = 0; col < 80; col++)
    {
        for (int lig = 0; lig < 25; lig++)
        {
            ecrit_car(lig, col, ' ');
        }
    }
}

void defilement(void)
{
    memmove(ptr_mem(0, 0), ptr_mem(1, 0), 2 * 24 * 80);
    for (int i = 0; i < 80; i++)
    {
        ecrit_car(24, i, ' ');
    }
}

void traite_car(char c)
{
    if (32 <= c && c <= 126)
    {
        ecrit_car(x, y, c);
        if (y > 78)
        {
            if (x == 25)
            {
                defilement();
                place_curseur(24, 0);
            }
            else
            {
                place_curseur(x + 1, 0);
            }
        }
        else
        {
            place_curseur(x, y + 1);
        }
    }
    else if (c == 8)
    {
        if (y != 0)
        {
            place_curseur(x, y - 1);
        }
    }
    else if (c == 9)
    {
        if (y < 79)
        {
            place_curseur(x, y + (8 - (y % 8)));
        }
    }
    else if (c == 10)
    {
        if (x == 24)
        {
            defilement();
            place_curseur(24, 0);
        }
        else
        {
            place_curseur(x + 1, 0);
        }
    }
    else if (c == 12)
    {
        efface_ecran();
        place_curseur(0, 0);
    }
    else if (c == 13)
    {
        place_curseur(x, 0);
    }
}

void console_putbytes(char *chaine, int32_t taille)
{
    // On écrit la chaine caractère par caractère
    for (int i = 0; i < taille; i++)
    {
        traite_car(chaine[i]);
    }
}
