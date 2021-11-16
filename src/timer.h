
#pragma once

#include "memory.h"

struct timer
{
  uint8_t tima_started;
  uint8_t last_pulse;
  uint8_t overflow;
};

void timer_init(struct timer *timer);
void timer_cycle(struct timer *timer, struct memory *mem, uint32_t cycles);
