#ifndef HORLOGE_H_
#define HORLOGE_H_

#include "stdint.h"

void init_idt();
void init_traitant_IT(int32_t num_IT, void (*traitant)(void));
void tic_PIT(void);
void show_time(const char *chaine);
void clock_settings(unsigned long *quartz, unsigned long *ticks);
void masque_IRQ(uint32_t num_IRQ, bool masque);
unsigned long current_clock();
//void wait_clock(unsigned long clock);

#endif /* HORLOGE_H_ */
