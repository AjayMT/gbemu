
#include <string.h>
#include <stdio.h>
#include "cpu.h"

void cpu_init(struct cpu *cpu)
{
  memset(cpu, 0, sizeof(struct cpu));
}

void debug_dump_regs(struct cpu cpu)
{
  printf(
    "AF = %hx\nBC = %hx\nDE = %hx\nHL = %hx\n",
    cpu.regs.af, cpu.regs.bc, cpu.regs.de, cpu.regs.hl
    );
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

  uint8_t upper = (a & 0xF0) >> 4;
  uint8_t lower = a & 0xF;
  uint8_t *dests[] = {
    (uint8_t *)&cpu->regs.bc,       // c
    (uint8_t *)&cpu->regs.de,       // e
    (uint8_t *)&cpu->regs.hl,       // l
    ((uint8_t *)&cpu->regs.af) + 1, // a
    ((uint8_t *)&cpu->regs.bc) + 1, // b
    ((uint8_t *)&cpu->regs.de) + 1, // d
    ((uint8_t *)&cpu->regs.hl) + 1  // h
  };
  uint8_t sources[] = {
    (cpu->regs.bc & 0xFF00) >> 8, // b
    cpu->regs.bc & 0xFF,          // c
    (cpu->regs.de & 0xFF00) >> 8, // d
    cpu->regs.de & 0xFF,          // e
    (cpu->regs.hl & 0xFF00) >> 8, // h
    cpu->regs.hl & 0xFF,          // l
    0,                            // <none>
    (cpu->regs.af & 0xFF00) >> 8  // a
  };

  // ld rr, nn
  if (lower == 1 && upper < 4)
  {
    uint16_t *dests[] = { &cpu->regs.bc, &cpu->regs.de, &cpu->regs.hl, &cpu->regs.sp };
    *dests[upper] = (c << 8) | b;
    cpu->regs.pc += 3;
    cpu->clock += 12;
    return;
  }

  // ld r, r
  if ((lower < 6 || lower == 7) && upper >= 4 && upper <= 6)
  {
    *dests[upper] = sources[lower];
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }
  if (((lower >= 8 && lower <= 0xD) || lower == 0xF) && upper >= 4 && upper <= 7)
  {
    *dests[upper - 4] = sources[lower - 8];
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }

  // ld r, (hl)
  if (((lower == 6 && upper <= 6) || (lower == 0xE && upper <= 7)) && upper >= 4)
  {
    if (lower == 6)
      *dests[upper] = memory_read(mem, cpu->regs.hl);
    else if (lower == 0xE)
      *dests[upper - 4] = memory_read(mem, cpu->regs.hl);
    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }

  // add a, r
  if ((lower <= 5 || lower == 7) && upper == 8)
  {
    uint8_t *dest = ((uint8_t *)&cpu->regs.af) + 1;
    uint16_t orig = *dest;
    *dest += sources[lower];
    cpu->regs.af &= ~0b100;
    if (orig + sources[lower] > 0xFF) cpu->regs.af |= 1; // carry flag
    if ((orig & 0xF) + (sources[lower] & 0xF) > 0xF) cpu->regs.af |= 0b10; // half-carry flag
    if (*dest == 0) cpu->regs.af |= 0b1000;
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }
}
