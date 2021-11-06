
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

void add_to_register_a(struct cpu *cpu, uint8_t value, uint8_t carry)
{
  uint8_t *dest = ((uint8_t *)&cpu->regs.af) + 1;
  uint16_t orig = *dest;
  *dest += value + carry;
  cpu->regs.af &= ~0b100;
  if (orig + value + carry > 0xFF) cpu->regs.af |= 1; // carry flag
  else cpu->regs.af &= ~1;
  if ((orig & 0xF) + (value & 0xF) + carry > 0xF) cpu->regs.af |= 0b10; // half-carry flag
  else cpu->regs.af &= ~0b10;
  if (*dest == 0) cpu->regs.af |= 0b1000; // zero flag
  else cpu->regs.af &= ~0b1000;
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
    add_to_register_a(cpu, sources[lower], 0);
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }

  // add a, (hl)
  if (a == 0x86)
  {
    add_to_register_a(cpu, memory_read(mem, cpu->regs.hl), 0);
    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }

  // adc a, r
  if (((lower >= 8 && lower <= 0xD) || lower == 0xF) && upper == 8)
  {
    add_to_register_a(cpu, sources[lower - 8], (cpu->regs.af & 1));
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }

  // adc a, (hl)
  if (a == 0x8E)
  {
    add_to_register_a(cpu, memory_read(mem, cpu->regs.hl), (cpu->regs.af & 1));
    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }
}
