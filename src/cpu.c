
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

  // ld rr, nn
  if ((a & 0x0F) == 1 && ((a & 0xF0) >> 4) < 4)
  {
    uint16_t *dests[] = { &cpu->regs.bc, &cpu->regs.de, &cpu->regs.hl, &cpu->regs.sp };
    *dests[(a & 0xF0) >> 4] = (c << 8) | b;
    cpu->regs.pc += 3;
    cpu->clock += 12;
    return;
  }
}
