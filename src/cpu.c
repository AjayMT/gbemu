
#include <string.h>
#include "cpu.h"

void cpu_init(struct cpu *cpu)
{
  memset(cpu, 0, sizeof(struct cpu));
}

void run_prefix_cb_instruction(struct cpu *cpu, struct memory *mem, uint8_t a, uint8_t b)
{
  (void)cpu;
  (void)mem;
  (void)a;
  (void)b;
}

void cpu_run_instruction(struct cpu *cpu, struct memory *mem, uint8_t a, uint8_t b, uint8_t c)
{
  if (a == 0xCB)
  {
    run_prefix_cb_instruction(cpu, mem, a, b);
    return;
  }

  switch (a & 0x0F)
  {
  case 1:
  {
    switch (a & 0xF0)
    {
    case 0:
    {
      cpu->regs.bc = (c << 8) | b;
      cpu->regs.pc += 3;
      cpu->clock += 12;
      return;
    }
    }
  }
  }
}
