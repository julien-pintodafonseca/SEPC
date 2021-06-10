#ifndef HORLOGE_H_
#define HORLOGE_H_

#include "stdint.h"

extern void traitant_IT_32(void);

void init_traitant_IT(int32_t num_IT, void (*traitant)(void));
void clock_settings(unsigned long *quartz, unsigned long *ticks);
void tic_PIT(void);
void print_time(const char *chaine);
void wait_clock(unsigned long clock);
void masque_IRQ(uint32_t num_IRQ, bool masque);
unsigned long current_clock();

#endif /* HORLOGE_H_ */
