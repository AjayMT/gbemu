
#include <string.h>
#include "timer.h"

void timer_init(struct timer *timer)
{
  memset(timer, 0, sizeof(struct timer));
}

uint32_t get_timer_frequency(struct memory *mem)
{
  uint8_t f = memory_read(mem, ADDR_REG_TAC) & FLAG_TIMER_CLOCK_MODE;
  switch (f)
  {
  case 0: return 4096;
  case 1: return 262144;
  case 2: return 65536;
  case 3: return 16384;
  }
  return 0;
}

void handle_overflow(struct timer *timer, struct memory *mem)
{
  if (!timer->overflow) return;
  timer->overflow = 0;
  memory_write(mem, ADDR_REG_TIMA, memory_read(mem, ADDR_REG_TMA));
  mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_TIMER;
}

void handle_tima(struct timer *timer, struct memory *mem, uint32_t cycles, uint16_t clock)
{
  if (!(memory_read(mem, ADDR_REG_TAC) & FLAG_TIMER_START))
  {
    timer->tima_started = 0;
    return;
  }

  if (!(memory_read(mem, ADDR_REG_TMA) & FLAG_TIMER_START)) return;

  uint32_t prev_clock = clock - cycles;
  if (!timer->tima_started)
  {
    timer->tima_started = 1;
    prev_clock = clock - 1;
  }

  uint16_t bit = (4194304 / get_timer_frequency(mem)) >> 1;
  for (uint32_t i = prev_clock + 1; i <= clock; ++i)
  {
    handle_overflow(timer, mem);
    uint8_t pulse = i & bit;
    if (timer->last_pulse && !pulse)
    {
      uint8_t timer_value = memory_read(mem, ADDR_REG_TIMA) + 1;
      timer->overflow = timer_value == 0;
      memory_write(mem, ADDR_REG_TIMA, timer_value);
    }
    timer->last_pulse = pulse;
  }
}

void timer_cycle(struct timer *timer, struct memory *mem, uint32_t cycles)
{
  uint16_t internal_clock = (memory_read(mem, ADDR_REG_DIVIDER) << 8)
    | memory_read(mem, ADDR_REG_INTERNAL_CLOCK_LOW);
  internal_clock += cycles;
  mem->memory[ADDR_REG_DIVIDER] = (internal_clock & 0xFF00) >> 8;
  mem->memory[ADDR_REG_INTERNAL_CLOCK_LOW] = internal_clock & 0xFF;
  handle_tima(timer, mem, cycles, internal_clock);
}
