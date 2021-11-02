
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

  // ld rr, nn
  if ((a & 0x0F) == 1 && ((a & 0xF0) >> 4) < 4)
  {
    uint16_t *dests[] = { &cpu->regs.bc, &cpu->regs.de, &cpu->regs.hl, &cpu->regs.sp };
    *dests[(a & 0xF0) >> 4] = (c << 8) | b;
    cpu->regs.pc += 3;
    cpu->clock += 12;
    return;
  }

  // ld r, r
  if (((a & 0x0F) < 6 || (a & 0x0F) == 7) && ((a & 0xF0) >> 4) >= 4 && ((a & 0xF0) >> 4) <= 6)
  {
    uint8_t *dests[] = {
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
    *dests[((a & 0xF0) >> 4) - 4] = sources[a & 0x0F];
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }
  if (
    ((a & 0x0F) - 8 < 6 || (a & 0x0F) == 0x0F) && ((a & 0xF0) >> 4) >= 4 && ((a & 0xF0) >> 4) <= 7
    )
  {
    uint8_t *dests[] = {
      (uint8_t *)&cpu->regs.bc,       // c
      (uint8_t *)&cpu->regs.de,       // e
      (uint8_t *)&cpu->regs.hl,       // l
      ((uint8_t *)&cpu->regs.af) + 1, // a
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
    *dests[((a & 0xF0) >> 4) - 4] = sources[(a & 0x0F) - 8];
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }
}
