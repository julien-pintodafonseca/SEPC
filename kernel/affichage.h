#ifndef AFFICHAGE_H_
#define AFFICHAGE_H_

#include "stdint.h"

uint32_t getx(void);
uint32_t gety(void);
uint16_t *ptr_mem(uint32_t lig, uint32_t col);
void ecrit_car(uint32_t lig, uint32_t col, char c);
void place_curseur(uint32_t lig, uint32_t col);
void efface_ecran(void);
void defilement(void);
void traite_car(char c);

#endif /* AFFICHAGE_H_ */
