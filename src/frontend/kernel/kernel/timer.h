#ifndef TIMER_H
#define TIMER_H

#include "core.h"

void init_timer(void);
uint32_t timer_ticks(void);
void timer_sleep(uint32_t ms);

#endif
