
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

void add_to_register_a(struct cpu *cpu, uint8_t value, uint8_t secondary_function)
{
  uint8_t carry = 0;
  if (secondary_function) carry = (cpu->regs.af & (1 << 4)) >> 4;
  uint8_t *dest = ((uint8_t *)&cpu->regs.af) + 1;
  uint16_t orig = *dest;
  *dest += value + carry;

  cpu->regs.af &= ~0xFF;
  if (orig + value + carry > 0xFF) cpu->regs.af |= 1 << 4; // carry flag
  if ((orig & 0xF) + (value & 0xF) + carry > 0xF) cpu->regs.af |= 1 << 5; // half-carry flag
  if (*dest == 0) cpu->regs.af |= 1 << 7; // zero flag
}

void sub_from_register_a(struct cpu *cpu, uint8_t value, uint8_t secondary_function)
{
  uint8_t carry = 0;
  if (secondary_function) carry = (cpu->regs.af & (1 << 4)) >> 4;
  uint8_t *dest = ((uint8_t *)&cpu->regs.af) + 1;
  uint16_t orig = *dest;
  *dest -= (value + carry);

  cpu->regs.af &= ~0xFF;
  cpu->regs.af |= (1 << 6);
  if (orig < (uint16_t)value + carry) cpu->regs.af |= 1 << 4; // carry flag
  if ((orig & 0xF) < (uint16_t)(value & 0xF) + carry) cpu->regs.af |= 1 << 5; // half-carry flag
  if (*dest == 0) cpu->regs.af |= 1 << 7; // zero flag
}

void and_or_xor_register_a(struct cpu *cpu, uint8_t value, uint8_t xor)
{
  uint8_t *f = (uint8_t *)&cpu->regs.af;
  uint8_t *a = f + 1;
  if (xor)
  {
    *a ^= value;
    *f = 0;
  }
  else
  {
    *a &= value;
    *f = 1 << 5;
  }
  if (*a == 0) *f |= 1 << 7;
}

void or_or_cp_register_a(struct cpu *cpu, uint8_t value, uint8_t cp)
{
  uint8_t *f = (uint8_t *)&cpu->regs.af;
  uint8_t *a = f + 1;
  if (cp)
  {
    *f = 1 << 6;
    if (*a == value) *f |= 1 << 7; // zero flag
    if (*a < value) *f |= 1 << 4; // carry flag
    if ((*a & 0xF) < (value & 0xF)) *f |= 1 << 5; // half-carry flag
  }
  else
  {
    *a |= value;
    if (*a == 0) *f = 1 << 7;
    else *f = 0;
  }
}

void inc_register_or_value(struct cpu *cpu, uint8_t *dest)
{
  (*dest)++;
  cpu->regs.af &= (uint8_t)(~0b1110) << 4;
  if (*dest == 0) cpu->regs.af |= 1 << 7; // zero flag
  if ((*dest & 0xF) == 0) cpu->regs.af |= 1 << 5; // half-carry flag
}

void dec_register_or_value(struct cpu *cpu, uint8_t *dest)
{
  (*dest)--;
  cpu->regs.af &= (uint8_t)(~0b1110) << 4;
  cpu->regs.af |= 1 << 6;
  if (*dest == 0) cpu->regs.af |= 1 << 7; // zero flag
  if ((*dest & 0xF) == 0xF) cpu->regs.af |= 1 << 5; // half-carry flag
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

  // nop
  if (a == 0)
  {
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }

  // ld rr nn
  if (lower == 1 && upper < 4)
  {
    uint16_t *dests[] = { &cpu->regs.bc, &cpu->regs.de, &cpu->regs.hl, &cpu->regs.sp };
    *dests[upper] = (c << 8) | b;
    cpu->regs.pc += 3;
    cpu->clock += 12;
    return;
  }

  // ld r r
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

  // ld r (hl)
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

  // ld (hl) r
  if (upper == 7 && (lower <= 5 || lower == 7))
  {
    memory_write(mem, cpu->regs.hl, sources[lower]);
    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }

  // add/adc/sub/sbc/and/xor/or/cp
  void (*arithmetic_logic_operations[])(struct cpu *, uint8_t, uint8_t) = {
    add_to_register_a,
    sub_from_register_a,
    and_or_xor_register_a,
    or_or_cp_register_a
  };
  if (upper >= 8 && upper <= 0xB)
  {
    uint8_t value;
    if (lower >= 8) value = sources[lower - 8];
    else value = sources[lower];

    uint8_t mem_read = lower == 6 || lower == 0xE;
    if (mem_read) value = memory_read(mem, cpu->regs.hl);

    arithmetic_logic_operations[upper - 8](cpu, value, lower >= 8);

    cpu->regs.pc++;
    cpu->clock += mem_read ? 8 : 4;
    return;
  }
  if (upper >= 0xC && (lower == 6 || lower == 0xE))
  {
    arithmetic_logic_operations[upper - 0xC](cpu, b, lower == 0xE);
    cpu->regs.pc += 2;
    cpu->clock += 8;
    return;
  }

  // inc r/(hl), dec r/(hl)
  if (upper <= 3 && (lower == 0xC || lower == 0xD))
  {
    if (lower == 0xC) inc_register_or_value(cpu, dests[upper]);
    else dec_register_or_value(cpu, dests[upper]);
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }
  if (upper <= 3 && (lower == 4 || lower == 5))
  {
    uint8_t mem_read = upper == 3;
    uint8_t value;
    if (mem_read) value = memory_read(mem, cpu->regs.hl);
    else value = *dests[upper + 4];

    if (lower == 4) inc_register_or_value(cpu, &value);
    else dec_register_or_value(cpu, &value);

    if (mem_read) memory_write(mem, cpu->regs.hl, value);
    else *dests[upper + 4] = value;

    cpu->regs.pc++;
    cpu->clock += mem_read ? 12 : 4;
    return;
  }

  // ld r/(hl) d8
  if (upper <= 3 && (lower == 6 || lower == 0xE))
  {
    uint8_t dest_idx = upper;
    if (lower == 6) dest_idx += 4;

    uint8_t mem_read = upper == 3 && lower == 6;
    if (mem_read) memory_write(mem, cpu->regs.hl, b);
    else *dests[dest_idx] = b;

    cpu->clock += mem_read ? 12 : 8;
    cpu->regs.pc += 2;
    return;
  }

  // inc rr, dec rr
  if (upper <= 3 && (lower == 3 || lower == 0xB))
  {
    uint16_t *dests[] = { &cpu->regs.bc, &cpu->regs.de, &cpu->regs.hl, &cpu->regs.sp };
    if (lower == 3) (*dests[upper])++;
    else (*dests[upper])--;
    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }

  // ld (rr) a
  if (upper <= 3 && lower == 2)
  {
    uint16_t addresses[] = { cpu->regs.bc, cpu->regs.de, cpu->regs.hl, cpu->regs.hl };
    memory_write(mem, addresses[upper], cpu->regs.af >> 8);
    if (upper == 2) cpu->regs.hl++;
    else if (upper == 3) cpu->regs.hl--;
    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }

  // ld a (rr)
  if (upper <= 3 && lower == 0xA)
  {
    uint16_t addresses[] = { cpu->regs.bc, cpu->regs.de, cpu->regs.hl, cpu->regs.hl };
    uint8_t *a = ((uint8_t *)&cpu->regs.af) + 1;
    *a = memory_read(mem, addresses[upper]);
    if (upper == 2) cpu->regs.hl++;
    else if (upper == 3) cpu->regs.hl--;
    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }

  // add hl rr
  if (upper <= 3 && lower == 9)
  {
    uint16_t sources[] = { cpu->regs.bc, cpu->regs.de, cpu->regs.hl, cpu->regs.sp };
    uint32_t orig = cpu->regs.hl;
    cpu->regs.hl += sources[upper];

    cpu->regs.af &= (uint8_t)(~0b111) << 4;
    if (orig + sources[upper] > 0xFFFF) cpu->regs.af |= 1 << 4; // carry flag
    // checking if there was a half-carry from bit 11 to bit 12
    if (((orig & 0xFFFF) ^ sources[upper] ^ ((orig + sources[upper]) & 0xFFFF)) & 0x1000)
      cpu->regs.af |= 1 << 5; // half-carry flag

    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }
}
