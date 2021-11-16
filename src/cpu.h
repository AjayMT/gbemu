
#pragma once

#include <stdint.h>
#include "memory.h"

struct cpu_registers
{
  uint16_t af;
  uint16_t bc;
  uint16_t de;
  uint16_t hl;
  uint16_t sp;
  uint16_t pc;
};

struct cpu
{
  struct cpu_registers regs;
  uint32_t clock;
  uint8_t interrupts_enabled;
  uint8_t halted;
};

void cpu_init(struct cpu *cpu);
void cpu_run_instruction(struct cpu *cpu, struct memory *mem, uint8_t a, uint8_t b, uint8_t c);
void cpu_handle_interrupts(struct cpu *cpu, struct memory *mem);
