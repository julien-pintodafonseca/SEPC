#ifndef HORLOGE_H_
#define HORLOGE_H_

#include "stdint.h"

unsigned long current_clock();
void show_time(const char *chaine);
void init_traitant_IT(int32_t num_IT, void (*traitant)(void));
extern void traitant_IT_32(void);
void tic_PIT(void);
void clock_settings(unsigned long *quartz, unsigned long *ticks);
void print_time(const char *chaine);
void masque_IRQ(uint32_t num_IRQ, bool masque);
void wait_clock(unsigned long clock);

#endif /* HORLOGE_H_ */
