/* Phase 1 : Affichage à l'écran */
/*   Fonction console_putbytes   */

#include "stdint.h"
#include "cpu.h"
#include "string.h"
#include "stdio.h"

#define NB_LIG 25
#define NB_COL 80

uint32_t x = 0; // ligne
uint32_t y = 0; // colonne

/* 
    Retourne un pointeur sur la zone mémoire correspondant
    à la case de la console ligne lig et colonne col
*/
uint16_t *ptr_mem(uint32_t lig, uint32_t col)
{
    return (uint16_t *)(0xB8000 + 2 * (lig * NB_COL + col));
}

/* Ecrit un caractère c à la ligne lig et la colonne col */
void ecrit_car(uint32_t lig, uint32_t col, char c)
{
    uint16_t *addr = ptr_mem(lig, col);
    uint8_t format = (0 << 7) | (0 << 4) | (15); // Ecriture en blanc sur noir non clignotant
    *addr = (format << 8) | c;
}

/* Déplace le curseur à la ligne lig et la colonne col */
void place_curseur(uint32_t lig, uint32_t col)
{
    uint16_t pos = col + lig * NB_COL;
    // Ecriture de la partie basse
    outb(0x0F, 0x3D4);
    outb(pos, 0x3D5);
    // Ecriture de la partie haute
    outb(0x0E, 0x3D4);
    outb(pos >> 8, 0x3D5);
    // Màj des valeurs courantes du curseur
    x = lig;
    y = col;
}

/* Effacer l'écran */
void efface_ecran(void)
{
    memset(ptr_mem(0, 0), 0, 2 * NB_LIG * NB_COL);
    place_curseur(0, 0);
}

/* Défilement de l'écran */
void defilement(void)
{
    memmove(ptr_mem(0, 0), ptr_mem(1, 0), 2 * (NB_LIG - 1) * NB_COL);
    memset(ptr_mem((NB_LIG - 1), 0), 0, 2 * (NB_COL));
}

/* 
    Traitement d'un caractère c à afficher dans la console 
    de 32 à 126 : caractère classique à afficher
    8   : \b : décaler le curseur d'une case à gauche
    9   : \t : tabulation
    10  : \n : retour ligne
    12  : \f : ràz l'affichage
    13  : \r : décaler le curseur en début de ligne courante
    Les autres caractères sont ignorés
*/
void traite_car(char c)
{
    if (32 <= c && c <= 126)
    {
        ecrit_car(x, y, c);
        if (y >= (NB_COL - 1))
        {
            if (x >= NB_LIG)
            {
                defilement();
                place_curseur((NB_LIG - 1), 0);
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
        if (y < (NB_COL - 5))
        {
            place_curseur(x, y + (8 - (y % 8)));
        }
    }
    else if (c == 10)
    {
        if (x >= (NB_LIG - 1))
        {
            defilement();
            place_curseur((NB_LIG - 1), 0);
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

/* Ecrit la chaîne de caractère chaine de taille taille dans la console */
void console_putbytes(char *chaine, int32_t taille)
{
    // On écrit la chaine caractère par caractère
    for (int i = 0; i < taille; i++)
    {
        traite_car(chaine[i]);
    }
}

void show_time(const char *chaine)
{
    uint32_t x = NB_COL - strlen(chaine);
    uint32_t y = 0;
    place_curseur(y, x);
    printf("%s", chaine);
}