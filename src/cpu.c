
#include <stdlib.h>
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
  cpu->regs.af &= ~0b11100000;
  if (*dest == 0) cpu->regs.af |= 1 << 7; // zero flag
  if ((*dest & 0xF) == 0) cpu->regs.af |= 1 << 5; // half-carry flag
}

void dec_register_or_value(struct cpu *cpu, uint8_t *dest)
{
  (*dest)--;
  cpu->regs.af &= ~0b11100000;
  cpu->regs.af |= 1 << 6;
  if (*dest == 0) cpu->regs.af |= 1 << 7; // zero flag
  if ((*dest & 0xF) == 0xF) cpu->regs.af |= 1 << 5; // half-carry flag
}

void push(struct cpu *cpu, struct memory *mem, uint16_t value)
{
  uint8_t lower_byte = value;
  uint8_t upper_byte = value >> 8;
  cpu->regs.sp -= 2;
  memory_write(mem, cpu->regs.sp, lower_byte);
  memory_write(mem, cpu->regs.sp + 1, upper_byte);
}

uint16_t pop(struct cpu *cpu, struct memory *mem)
{
  uint8_t lower_byte = memory_read(mem, cpu->regs.sp);
  uint8_t upper_byte = memory_read(mem, cpu->regs.sp + 1);
  cpu->regs.sp += 2;
  return (uint16_t)lower_byte | (uint16_t)(upper_byte << 8);
}

uint8_t rotate_left(struct cpu *cpu, uint8_t value)
{
  uint8_t result = (value << 1) | ((value >> 7) & 1);
  cpu->regs.af &= ~0b11110000;
  if (result == 0) cpu->regs.af |= 1 << 7;
  if ((value >> 7) & 1) cpu->regs.af |= 1 << 4;
  return result;
}

uint8_t rotate_left_through_carry(struct cpu *cpu, uint8_t value)
{
  uint8_t carry = (cpu->regs.af & (1 << 4)) >> 4;
  uint8_t result = (value << 1) | carry;
  cpu->regs.af &= ~0b11110000;
  if (result == 0) cpu->regs.af |= 1 << 7;
  if ((value >> 7) & 1) cpu->regs.af |= 1 << 4;
  return result;
}

uint8_t rotate_right(struct cpu *cpu, uint8_t value)
{
  uint8_t result = (value >> 1) | ((value & 1) << 7);
  cpu->regs.af &= ~0b11110000;
  if (result == 0) cpu->regs.af |= 1 << 7;
  if (value & 1) cpu->regs.af |= 1 << 4;
  return result;
}

uint8_t rotate_right_through_carry(struct cpu *cpu, uint8_t value)
{
  uint8_t carry = (cpu->regs.af & (1 << 4)) >> 4;
  uint8_t result = (value >> 1) | (carry << 7);
  cpu->regs.af &= ~0b11110000;
  if (result == 0) cpu->regs.af |= 1 << 7;
  if (value & 1) cpu->regs.af |= 1 << 4;
  return result;
}

uint8_t shift_left_arithmetic(struct cpu *cpu, uint8_t value)
{
  uint8_t result = value << 1;
  cpu->regs.af &= ~0b11110000;
  if (value & (1 << 7)) cpu->regs.af |= 1 << 4;
  if (result == 0) cpu->regs.af |= 1 << 7;
  return result;
}

uint8_t shift_right_arithmetic(struct cpu *cpu, uint8_t value)
{
  uint8_t result = (value >> 1) | (value & (1 << 7));
  cpu->regs.af &= ~0b11110000;
  if (value & 1) cpu->regs.af |= 1 << 4;
  if (result == 0) cpu->regs.af |= 1 << 7;
  return result;
}

uint8_t swap(struct cpu *cpu, uint8_t value)
{
  uint8_t result = ((value & 0xF) << 4) | ((value & 0xF0) >> 4);
  cpu->regs.af &= ~0b11110000;
  if (result == 0) cpu->regs.af |= 1 << 7;
  return result;
}

uint8_t shift_right_logical(struct cpu *cpu, uint8_t value)
{
  uint8_t result = value >> 1;
  cpu->regs.af &= ~0b11110000;
  if (value & 1) cpu->regs.af |= 1 << 4;
  if (result == 0) cpu->regs.af |= 1 << 7;
  return result;
}

uint8_t rotate_shift_swap(
  struct cpu *cpu, uint8_t first_function, uint8_t second_function, uint8_t value
  )
{
  uint8_t (*functions[])(struct cpu *, uint8_t) = {
    rotate_left, rotate_right,
    rotate_left_through_carry, rotate_right_through_carry,
    shift_left_arithmetic, shift_right_arithmetic,
    swap, shift_right_logical
  };
  return functions[(first_function << 1) | second_function](cpu, value);
}

uint8_t bit(struct cpu *cpu, uint8_t even, uint8_t odd, uint8_t value)
{
  uint8_t bit = (even * 2) | odd;
  cpu->regs.af &= ~(0b11100000);
  cpu->regs.af |= 1 << 5;
  if (((value >> bit) & 1) == 0) cpu->regs.af |= 1 << 7;
  return value;
}

uint8_t res(struct cpu *cpu, uint8_t even, uint8_t odd, uint8_t value)
{
  (void)cpu;
  uint8_t bit = (even * 2) | odd;
  return value & (~(1 << bit));
}

uint8_t set(struct cpu *cpu, uint8_t even, uint8_t odd, uint8_t value)
{
  (void)cpu;
  uint8_t bit = (even * 2) | odd;
  return value | (1 << bit);
}

void run_prefix_cb_instruction(struct cpu *cpu, struct memory *mem, uint8_t opcode)
{
  uint8_t upper = (opcode & 0xF0) >> 4;
  uint8_t lower = opcode & 0xF;

  uint8_t *dests[] = {
    ((uint8_t *)&cpu->regs.bc) + 1, // b
    (uint8_t *)&cpu->regs.bc,       // c
    ((uint8_t *)&cpu->regs.de) + 1, // d
    (uint8_t *)&cpu->regs.de,       // e
    ((uint8_t *)&cpu->regs.hl) + 1, // h
    (uint8_t *)&cpu->regs.hl,       // l
    0,                              // <none>
    ((uint8_t *)&cpu->regs.af) + 1, // a
  };

  uint8_t (*functions[])(struct cpu *, uint8_t, uint8_t, uint8_t) = {
    rotate_shift_swap, bit, res, set
  };

  uint8_t value = 0;
  uint8_t mem_op = dests[(lower & 7)] == 0;
  if (mem_op) value = memory_read(mem, cpu->regs.hl);
  else value = *dests[lower & 7];

  value = functions[upper >> 2](cpu, upper & 3, lower >> 3, value);

  if ((upper >> 2) == 1) // bit
  {
    cpu->regs.pc++;
    cpu->clock += mem_op ? 8 : 4;
    return;
  }

  if (mem_op) memory_write(mem, cpu->regs.hl, value);
  else *dests[lower & 7] = value;

  cpu->regs.pc++;
  cpu->clock += mem_op ? 12 : 4;
}

void cpu_run_instruction(struct cpu *cpu, struct memory *mem, uint8_t a, uint8_t b, uint8_t c)
{
  if (a == 0xCB)
  {
    cpu->regs.pc++;
    cpu->clock += 4;
    run_prefix_cb_instruction(cpu, mem, b);
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

  // add, adc, sub, sbc, and, xor, or, cp
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

    cpu->regs.af &= ~0b1110000;
    if (orig + sources[upper] > 0xFFFF) cpu->regs.af |= 1 << 4; // carry flag
    // checking if there was a half-carry from bit 11 to bit 12
    if (((orig & 0xFFFF) ^ sources[upper] ^ ((orig + sources[upper]) & 0xFFFF)) & 0x1000)
      cpu->regs.af |= 1 << 5; // half-carry flag

    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }

  // push/pop
  if (upper >= 0xC && (lower == 1 || lower == 5))
  {
    uint16_t *dests[] = { &cpu->regs.bc, &cpu->regs.de, &cpu->regs.hl, &cpu->regs.af };
    if (lower == 1)
    {
      if (upper - 0xC == 3) // pop AF is special
        *dests[upper - 0xC] = pop(cpu, mem) & 0xFFF0;
      else *dests[upper - 0xC] = pop(cpu, mem);
    }
    if (lower == 5) push(cpu, mem, *dests[upper - 0xC]);
    cpu->clock += lower == 1 ? 12 : 16;
    cpu->regs.pc++;
    return;
  }

  // rst
  if (upper >= 0xC && (lower == 7 || lower == 0xF))
  {
    uint16_t targets[] = { 0, 0x10, 0x20, 0x30 };
    push(cpu, mem, cpu->regs.pc + 1);
    cpu->regs.pc = targets[upper - 0xC] + (lower == 0xF ? 8 : 0);
    cpu->clock += 16;
    return;
  }

  // jp nz a16, jp nc a16, jp z a16, jp c a16
  if ((lower == 2 || lower == 0xA) && (upper == 0xC || upper == 0xD))
  {
    uint8_t flag_offset = upper == 0xC ? 7 : 4;
    uint8_t flag = (cpu->regs.af & (1 << flag_offset)) >> flag_offset;
    if (lower == 0xA) flag = flag == 0;
    if (flag)
    {
      cpu->regs.pc += 3;
      cpu->clock += 12;
      return;
    }
    cpu->regs.pc = (c << 8) | b;
    cpu->clock += 16;
    return;
  }

  // ld (0xFF00+c) a, ld a (0xFF00+c)
  if (lower == 2 && upper >= 0xE)
  {
    uint16_t address = 0xFF00 + (cpu->regs.bc & 0xFF);
    uint8_t *a = ((uint8_t *)&cpu->regs.af) + 1;
    if (upper == 0xE) memory_write(mem, address, *a);
    else *a = memory_read(mem, address);
    cpu->clock += 8;
    cpu->regs.pc++;
    return;
  }

  // ret nz, ret nc, ret z, ret c
  if ((lower == 0 || lower == 8) && (upper == 0xC || upper == 0xD))
  {
    uint8_t flag_offset = upper == 0xC ? 7 : 4;
    uint8_t flag = (cpu->regs.af & (1 << flag_offset)) >> flag_offset;
    if (lower == 8) flag = flag == 0;
    if (flag)
    {
      cpu->regs.pc++;
      cpu->clock += 8;
      return;
    }

    cpu->regs.pc = pop(cpu, mem);
    cpu->clock += 20;
    return;
  }

  // ldh (0xFF00 + a8) a, ldh a (0xFF00 + a8)
  if (lower == 0 && upper >= 0xE)
  {
    uint16_t address = 0xFF00 + b;
    uint8_t *a = ((uint8_t *)&cpu->regs.af) + 1;
    if (upper == 0xE) memory_write(mem, address, *a);
    else *a = memory_read(mem, address);
    cpu->regs.pc += 2;
    cpu->clock += 12;
    return;
  }

  // jr nz r8, jr nc r8, jr z r8, jr c r8
  if ((lower == 0 || lower == 8) && (upper == 2 || upper == 3))
  {
    uint8_t flag_offset = upper == 2 ? 7 : 4;
    uint8_t flag = (cpu->regs.af & (1 << flag_offset)) >> flag_offset;
    if (lower == 8) flag = flag == 0;
    if (flag)
    {
      cpu->regs.pc += 2;
      cpu->clock += 8;
      return;
    }

    cpu->regs.pc += (int8_t)b + 2;
    cpu->clock += 12;
    return;
  }

  // jp a16
  if (a == 0xC3)
  {
    cpu->regs.pc = (c << 8) | b;
    cpu->clock += 16;
    return;
  }

  // call nz a16, call nc a16, call z a16, call c a16
  if ((lower == 4 || lower == 0xC) && upper >= 0xC)
  {
    uint8_t flag_offset = upper == 0xC ? 7 : 4;
    uint8_t flag = (cpu->regs.af & (1 << flag_offset)) >> flag_offset;
    if (lower == 0xC) flag = flag == 0;
    if (flag)
    {
      cpu->regs.pc += 3;
      cpu->clock += 12;
      return;
    }

    push(cpu, mem, cpu->regs.pc + 3);
    cpu->regs.pc = b | (c << 8);
    cpu->clock += 24;
    return;
  }

  // add sp r8, ld hl sp+r8
  if (lower == 8 && upper >= 0xE)
  {
    int8_t sb = (int8_t)b;
    int32_t result = cpu->regs.sp + sb;
    cpu->regs.af &= ~0b11110000;
    // carry flag
    if (((cpu->regs.sp ^ sb ^ (result & 0xFFFF)) & 0x100) == 0x100) cpu->regs.af |= 1 << 4;
    // half-carry flag
    if (((cpu->regs.sp ^ sb ^ (result & 0xFFFF)) & 0x10) == 0x10) cpu->regs.af |= 1 << 5;

    if (upper == 0xE) cpu->regs.sp = (uint16_t)result;
    else cpu->regs.hl = (uint16_t)result;

    cpu->regs.pc += 2;
    cpu->clock += upper == 0xE ? 16 : 12;
    return;
  }

  // ret, reti
  if (lower == 9 && (upper == 0xC || upper == 0xD))
  {
    if (upper == 0xD) cpu->interrupts_enabled = 1;
    cpu->regs.pc = pop(cpu, mem);
    cpu->clock += 16;
    return;
  }

  // jp hl
  if (a == 0xE9)
  {
    cpu->regs.pc = cpu->regs.hl;
    cpu->clock += 4;
    return;
  }

  // ld sp hl
  if (a == 0xF9)
  {
    cpu->regs.sp = cpu->regs.hl;
    cpu->regs.pc++;
    cpu->clock += 8;
    return;
  }

  // ld (a16) a, ld a (a16)
  if (lower == 0xA && upper >= 0xE)
  {
    uint8_t *a = ((uint8_t *)&cpu->regs.af) + 1;
    uint16_t address = (c << 8) | b;
    if (upper == 0xE) memory_write(mem, address, *a);
    else *a = memory_read(mem, address);
    cpu->regs.pc += 3;
    cpu->clock += 16;
    return;
  }

  // jr r8
  if (a == 0x18)
  {
    cpu->regs.pc += (int8_t)b + 2;
    cpu->clock += 12;
    return;
  }

  // ld (a16) sp
  if (a == 8)
  {
    uint16_t address = (c << 8) | b;
    memory_write(mem, address, cpu->regs.sp & 0xFF);
    memory_write(mem, address + 1, cpu->regs.sp >> 8);
    cpu->regs.pc += 3;
    cpu->clock += 20;
    return;
  }

  // call a16
  if (a == 0xCD)
  {
    uint16_t address = (c << 8) | b;
    push(cpu, mem, cpu->regs.pc + 3);
    cpu->regs.pc = address;
    cpu->clock += 24;
    return;
  }

  // halt, di, ei
  if (a == 0x76 || a == 0xF3 || a == 0xFB)
  {
    if (a == 0x76) cpu->halted = 1;
    else if (a == 0xF3) cpu->interrupts_enabled = 0;
    else cpu->interrupts_enabled = 1;
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }

  // rlca, rla, rrca, rra
  if (upper <= 1 && (lower == 7 || lower == 0xF))
  {
    uint8_t *a = ((uint8_t *)&cpu->regs.af) + 1;
    *a = rotate_shift_swap(cpu, upper, lower == 0xF, *a);
    cpu->regs.af &= ~0b10000000;
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }

  // scf, ccf
  if (upper == 3 && (lower == 7 || lower == 0xF))
  {
    uint8_t carry = (cpu->regs.af & (1 << 4)) >> 4;
    if (lower == 7) carry = 1;
    else carry = carry == 0;
    cpu->regs.af &= ~0b1110000;
    if (carry) cpu->regs.af |= 1 << 4;
    else cpu->regs.af &= ~(1 << 4);
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }

  // cpl
  if (a == 0x2F)
  {
    uint8_t *a = ((uint8_t *)&cpu->regs.af) + 1;
    *a = ~(*a);
    cpu->regs.af |= 0b110 << 4;
    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }

  // daa
  if (a == 0x27)
  {
    // behavior: https://ehaskins.com/2018-01-30%20Z80%20DAA/
    uint8_t *a = ((uint8_t *)&cpu->regs.af) + 1;
    uint8_t reg = *a;
    uint16_t carry = (cpu->regs.af & (1 << 4)) >> 4;
    uint16_t half_carry = (cpu->regs.af & (1 << 5)) >> 5;
    uint16_t subtract = (cpu->regs.af & (1 << 6)) >> 6;
    uint16_t correction = carry ? 0x60 : 0;

    if (half_carry || (!subtract && ((reg & 0xF) > 9)))
      correction |= 6;

    if (carry || (!subtract && (reg > 0x99)))
      correction |= 0x60;

    if (subtract) reg = (uint8_t)(reg - correction);
    else reg = (uint8_t)(reg + correction);

    cpu->regs.af &= ~0b10110000;
    if (((correction << 2) & 0x100) != 0) cpu->regs.af |= 1 << 4;
    if (reg == 0) cpu->regs.af |= 1 << 7;
    *a = (uint8_t)reg;

    cpu->regs.pc++;
    cpu->clock += 4;
    return;
  }

  // stop
  if (a == 0x10)
  {
    printf("STOP instruction not implemented\n");
    debug_dump_regs(*cpu);
    while (1);
    return;
  }
}

void cpu_handle_interrupts(struct cpu *cpu, struct memory *mem)
{
  uint16_t interrupt_vector = 0;
  uint8_t interrupt_flag = 0;

  uint8_t vblank_interrupt = (memory_read(mem, ADDR_REG_INTERRUPT_ENABLED) & FLAG_INTERRUPT_VBLANK)
    && (memory_read(mem, ADDR_REG_INTERRUPT_FLAG) & FLAG_INTERRUPT_VBLANK);
  uint8_t lcd_interrupt = (memory_read(mem, ADDR_REG_INTERRUPT_ENABLED) & FLAG_INTERRUPT_LCD)
    && (memory_read(mem, ADDR_REG_INTERRUPT_FLAG) & FLAG_INTERRUPT_LCD);
  uint8_t timer_interrupt = (memory_read(mem, ADDR_REG_INTERRUPT_ENABLED) & FLAG_INTERRUPT_TIMER)
    && (memory_read(mem, ADDR_REG_INTERRUPT_FLAG) & FLAG_INTERRUPT_TIMER);
  uint8_t serial_interrupt = (memory_read(mem, ADDR_REG_INTERRUPT_ENABLED) & FLAG_INTERRUPT_SERIAL)
    && (memory_read(mem, ADDR_REG_INTERRUPT_FLAG) & FLAG_INTERRUPT_SERIAL);
  uint8_t input_interrupt = (memory_read(mem, ADDR_REG_INTERRUPT_ENABLED) & FLAG_INTERRUPT_INPUT)
    && (memory_read(mem, ADDR_REG_INTERRUPT_FLAG) & FLAG_INTERRUPT_INPUT);

  if (vblank_interrupt)
  {
    interrupt_vector = ADDR_VECTOR_VBLANK;
    interrupt_flag = FLAG_INTERRUPT_VBLANK;
  }
  else if (lcd_interrupt)
  {
    interrupt_vector = ADDR_VECTOR_LCD;
    interrupt_flag = FLAG_INTERRUPT_LCD;
  }
  else if (timer_interrupt)
  {
    interrupt_vector = ADDR_VECTOR_TIMER;
    interrupt_flag = FLAG_INTERRUPT_TIMER;
  }
  else if (serial_interrupt)
  {
    interrupt_vector = ADDR_VECTOR_SERIAL;
    interrupt_flag = FLAG_INTERRUPT_SERIAL;
  }
  else if (input_interrupt)
  {
    interrupt_vector = ADDR_VECTOR_INPUT;
    interrupt_flag = FLAG_INTERRUPT_INPUT;
  }

  if (interrupt_vector != 0) cpu->halted = 0;
  if (interrupt_vector == 0 || cpu->interrupts_enabled == 0) return;

  cpu->interrupts_enabled = 0;
  mem->memory[ADDR_REG_INTERRUPT_FLAG] &= ~interrupt_flag;
  push(cpu, mem, cpu->regs.pc);
  cpu->regs.pc = interrupt_vector;
}
