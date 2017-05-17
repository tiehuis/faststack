#ifndef TIMER_H
#define TIMER_H

#include "core.h"

void init_timer(void);
uint32_t timer_ticks(void);
uint32_t timer_seed(void);
void timer_sleep(uint32_t ms);

#endif
