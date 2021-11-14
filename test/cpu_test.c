
#include "../src/cpu.h"
#include "utest.h"

UTEST(cpu, nop)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 0, 0, 0);
  ASSERT_EQ(0, cpu.regs.af);
  ASSERT_EQ(0, cpu.regs.bc);
  ASSERT_EQ(0, cpu.regs.de);
  ASSERT_EQ(0, cpu.regs.hl);
  ASSERT_EQ(0, cpu.regs.sp);
  ASSERT_EQ(1, cpu.regs.pc);
  ASSERT_EQ((uint32_t)4, cpu.clock);
}

UTEST(cpu, ld_rr_d16)
{
  struct cpu cpu;
  cpu_init(&cpu);
  cpu_run_instruction(&cpu, NULL, 0x01, 0xad, 0xde);
  ASSERT_EQ(cpu.regs.bc, 0xdead);
  cpu_run_instruction(&cpu, NULL, 0x11, 0xad, 0xde);
  ASSERT_EQ(cpu.regs.de, 0xdead);
  cpu_run_instruction(&cpu, NULL, 0x21, 0xad, 0xde);
  ASSERT_EQ(cpu.regs.hl, 0xdead);
  cpu_run_instruction(&cpu, NULL, 0x31, 0xad, 0xde);
  ASSERT_EQ(cpu.regs.sp, 0xdead);
}

UTEST(cpu, ld_r_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 1;
  cpu_run_instruction(&cpu, NULL, 0x41, 0, 0); // ld b, c
  ASSERT_EQ(cpu.regs.bc, 0x101);
  cpu_run_instruction(&cpu, NULL, 0x51, 0, 0); // ld d, c
  ASSERT_EQ(cpu.regs.de, 0x100);

  cpu.regs.hl = 0x3333;
  cpu_run_instruction(&cpu, NULL, 0x44, 0, 0); // ld b, h
  ASSERT_EQ(cpu.regs.bc, 0x3301);
  cpu_run_instruction(&cpu, NULL, 0x54, 0, 0); // ld d, h
  ASSERT_EQ(cpu.regs.de, 0x3300);

  cpu.regs.af = 0x4141;
  cpu_run_instruction(&cpu, NULL, 0x67, 0, 0); // ld h, a
  ASSERT_EQ(cpu.regs.hl, 0x4133);

  cpu.regs.bc = 0x2021;
  cpu_run_instruction(&cpu, NULL, 0x68, 0, 0); // ld l, b
  ASSERT_EQ(cpu.regs.hl, 0x4120);

  cpu_run_instruction(&cpu, NULL, 0x5A, 0, 0); // ld e, d
  ASSERT_EQ(cpu.regs.de, 0x3333);

  cpu_run_instruction(&cpu, NULL, 0x7D, 0, 0); // ld a, l
  ASSERT_EQ(cpu.regs.af, 0x2041);

  cpu_run_instruction(&cpu, NULL, 0x4F, 0, 0); // ld c, a
  ASSERT_EQ(cpu.regs.bc, 0x2020);

  ASSERT_EQ(cpu.clock, (uint32_t)36);
  ASSERT_EQ(cpu.regs.pc, 9);
}

UTEST(cpu, ld_r_hl)
{
  struct memory mem;
  uint8_t zero = 0;
  memory_init(&mem, &zero, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xFFFF;
  cpu_run_instruction(&cpu, &mem, 0x7E, 0, 0); // ld a, (hl)
  ASSERT_EQ(cpu.regs.af, 0xFF);

  cpu.regs.bc = 0xFFFF;
  cpu_run_instruction(&cpu, &mem, 0x4E, 0, 0); // ld c, (hl)
  ASSERT_EQ(cpu.regs.bc, 0xFF00);

  cpu_run_instruction(&cpu, &mem, 0x46, 0, 0); // ld b, (hl)
  ASSERT_EQ(cpu.regs.bc, 0);

  ASSERT_EQ(cpu.clock, (uint32_t)24);
  ASSERT_EQ(cpu.regs.pc, 3);
}

UTEST(cpu, ld_hl_r)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, &mem, 0x77, 0, 0);
  ASSERT_EQ(0xFF, mem.memory[0]);

  cpu.regs.bc = 0xde;
  cpu_run_instruction(&cpu, &mem, 0x71, 0, 0);
  ASSERT_EQ(0xde, mem.memory[0]);

  ASSERT_EQ((uint32_t)16, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, add_a_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0;
  cpu.regs.bc = 3;
  cpu_run_instruction(&cpu, NULL, 0x81, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);
  ASSERT_EQ(3, cpu.regs.bc);

  cpu.regs.af = 0xF000;
  cpu_run_instruction(&cpu, NULL, 0x87, 0, 0);
  ASSERT_EQ(0xE000 | (1 << 4), cpu.regs.af);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, NULL, 0x87, 0, 0);
  ASSERT_EQ(0xFE00 | (3 << 4), cpu.regs.af);

  cpu.regs.af = 0xF000;
  cpu.regs.hl = 0x1000;
  cpu_run_instruction(&cpu, NULL, 0x84, 0, 0);
  ASSERT_EQ(9 << 4, cpu.regs.af);
  ASSERT_EQ(0x1000, cpu.regs.hl);

  ASSERT_EQ(4, cpu.regs.pc);
  ASSERT_EQ((uint32_t)16, cpu.clock);
}

UTEST(cpu, add_a_hl)
{
  struct memory mem;
  uint8_t value = 0;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0;
  value = 3;
  cpu_run_instruction(&cpu, &mem, 0x86, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  cpu.regs.af = 0xF000;
  value = 0xF0;
  cpu_run_instruction(&cpu, &mem, 0x86, 0, 0);
  ASSERT_EQ(0xE000 | (1 << 4), cpu.regs.af);

  cpu.regs.af = 0xFF00;
  value = 0xFF;
  cpu_run_instruction(&cpu, &mem, 0x86, 0, 0);
  ASSERT_EQ(0xFE00 | (3 << 4), cpu.regs.af);

  cpu.regs.af = 0xF000;
  value = 0x10;
  cpu_run_instruction(&cpu, &mem, 0x86, 0, 0);
  ASSERT_EQ(9 << 4, cpu.regs.af);

  ASSERT_EQ(4, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, add_a_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, NULL, 0xC6, 3, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  cpu.regs.af = 0xF000;
  cpu_run_instruction(&cpu, NULL, 0xC6, 0xF0, 0);
  ASSERT_EQ(0xE000 | (1 << 4), cpu.regs.af);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, NULL, 0xC6, 0xFF, 0);
  ASSERT_EQ(0xFE00 | (3 << 4), cpu.regs.af);

  cpu.regs.af = 0xF000;
  cpu_run_instruction(&cpu, NULL, 0xC6, 0x10, 0);
  ASSERT_EQ(9 << 4, cpu.regs.af);

  ASSERT_EQ(8, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, adc_a_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x600 | (1 << 4);
  cpu.regs.bc = 3;
  cpu_run_instruction(&cpu, NULL, 0x89, 0, 0);
  ASSERT_EQ(0xa00, cpu.regs.af);

  cpu.regs.af = 0xF000 | (1 << 4);
  cpu_run_instruction(&cpu, NULL, 0x8F, 0, 0);
  ASSERT_EQ(0xE100 | (1 << 4), cpu.regs.af);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, NULL, 0x8F, 0, 0);
  ASSERT_EQ(0xFE00 | (3 << 4), cpu.regs.af);

  cpu.regs.af = 0xF000 | (1 << 4);
  cpu.regs.hl = 0xF00;
  cpu_run_instruction(&cpu, NULL, 0x8C, 0, 0);
  ASSERT_EQ(11 << 4, cpu.regs.af);
  ASSERT_EQ(0xF00, cpu.regs.hl);

  ASSERT_EQ(4, cpu.regs.pc);
  ASSERT_EQ((uint32_t)16, cpu.clock);
}

UTEST(cpu, adc_a_hl)
{
  struct memory mem;
  uint8_t value = 0;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 4;
  value = 3;
  cpu_run_instruction(&cpu, &mem, 0x8E, 0, 0);
  ASSERT_EQ(0x400, cpu.regs.af);

  cpu.regs.af = 0xF000;
  value = 0xF0;
  cpu_run_instruction(&cpu, &mem, 0x8E, 0, 0);
  ASSERT_EQ(0xE000 | (1 << 4), cpu.regs.af);

  cpu.regs.af = 0xFF00 | (1 << 4);
  value = 0xFF;
  cpu_run_instruction(&cpu, &mem, 0x8E, 0, 0);
  ASSERT_EQ(0xFF00 | (3 << 4), cpu.regs.af);

  cpu.regs.af = 0xF000 | (1 << 4);
  value = 0xF;
  cpu_run_instruction(&cpu, &mem, 0x8E, 0, 0);
  ASSERT_EQ(11 << 4, cpu.regs.af);

  ASSERT_EQ(4, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, adc_a_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, NULL, 0xCE, 3, 0);
  ASSERT_EQ(0x400, cpu.regs.af);

  cpu.regs.af = 0xF000;
  cpu_run_instruction(&cpu, NULL, 0xCE, 0xF0, 0);
  ASSERT_EQ(0xE000 | (1 << 4), cpu.regs.af);

  cpu.regs.af = 0xFF00 | (1 << 4);
  cpu_run_instruction(&cpu, NULL, 0xCE, 0xFF, 0);
  ASSERT_EQ(0xFF00 | (3 << 4), cpu.regs.af);

  cpu.regs.af = 0xF000 | (1 << 4);
  cpu_run_instruction(&cpu, NULL, 0xCE, 0xF, 0);
  ASSERT_EQ(11 << 4, cpu.regs.af);

  ASSERT_EQ(8, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, sub_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  cpu.regs.bc = 1;
  cpu_run_instruction(&cpu, NULL, 0x91, 0, 0);
  ASSERT_EQ(0xE00 | (4 << 4), cpu.regs.af);
  ASSERT_EQ(1, cpu.regs.bc);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0x97, 0, 0);
  ASSERT_EQ(0xC << 4, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu.regs.hl = 0x200;
  cpu_run_instruction(&cpu, NULL, 0x94, 0, 0);
  ASSERT_EQ(0xFF00 | (7 << 4), cpu.regs.af);
  ASSERT_EQ(0x200, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, sub_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x96, 0, 0);
  ASSERT_EQ(0xE00 | (4 << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x96, 0, 0);
  ASSERT_EQ(0xC << 4, cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0x96, 0, 0);
  ASSERT_EQ(0xFF00 | (7 << 4), cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, sub_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  cpu_run_instruction(&cpu, NULL, 0xD6, 1, 0);
  ASSERT_EQ(0xE00 | (4 << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xD6, 1, 0);
  ASSERT_EQ(0xC << 4, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xD6, 2, 0);
  ASSERT_EQ(0xFF00 | (7 << 4), cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(6, cpu.regs.pc);
}

UTEST(cpu, sbc_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  cpu.regs.bc = 1;
  cpu_run_instruction(&cpu, NULL, 0x99, 0, 0);
  ASSERT_EQ(0xE00 | (4 << 4), cpu.regs.af);
  ASSERT_EQ(1, cpu.regs.bc);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0x9F, 0, 0);
  ASSERT_EQ(0xC << 4, cpu.regs.af);

  cpu.regs.af = 0x100 | (1 << 4);
  cpu.regs.hl = 0x100;
  cpu_run_instruction(&cpu, NULL, 0x9C, 0, 0);
  ASSERT_EQ(0xFF00 | (7 << 4), cpu.regs.af);
  ASSERT_EQ(0x100, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, sbc_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x9E, 0, 0);
  ASSERT_EQ(0xE00 | (4 << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x9E, 0, 0);
  ASSERT_EQ(0xC << 4, cpu.regs.af);

  cpu.regs.af = 0x100 | (1 << 4);
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x9E, 0, 0);
  ASSERT_EQ(0xFF00 | (7 << 4), cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, sbc_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  cpu_run_instruction(&cpu, NULL, 0xDE, 1, 0);
  ASSERT_EQ(0xE00 | (4 << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xDE, 1, 0);
  ASSERT_EQ(0xC << 4, cpu.regs.af);

  cpu.regs.af = 0x100 | (1 << 4);
  cpu_run_instruction(&cpu, NULL, 0xDE, 1, 0);
  ASSERT_EQ(0xFF00 | (7 << 4), cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(6, cpu.regs.pc);
}

UTEST(cpu, and_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x300;
  cpu.regs.bc = 2;
  cpu_run_instruction(&cpu, NULL, 0xA1, 0, 0);
  ASSERT_EQ(0x200 | (2 << 4), cpu.regs.af);
  ASSERT_EQ(2, cpu.regs.bc);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xA7, 0, 0);
  ASSERT_EQ(0x100 | (2 << 4), cpu.regs.af);

  cpu.regs.af = 0x200;
  cpu.regs.hl = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xA4, 0, 0);
  ASSERT_EQ(0b1010 << 4, cpu.regs.af);
  ASSERT_EQ(0x100, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, and_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x300;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0xA6, 0, 0);
  ASSERT_EQ(0x200 | (2 << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xA6, 0, 0);
  ASSERT_EQ(0x100 | (2 << 4), cpu.regs.af);

  cpu.regs.af = 0x200;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xA6, 0, 0);
  ASSERT_EQ(0b1010 << 4, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, and_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x300;
  cpu_run_instruction(&cpu, NULL, 0xE6, 2, 0);
  ASSERT_EQ(0x200 | (2 << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xE6, 1, 0);
  ASSERT_EQ(0x100 | (2 << 4), cpu.regs.af);

  cpu.regs.af = 0x200;
  cpu_run_instruction(&cpu, NULL, 0xE6, 1, 0);
  ASSERT_EQ(0b1010 << 4, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(6, cpu.regs.pc);
}

UTEST(cpu, xor_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x300;
  cpu.regs.bc = 2;
  cpu_run_instruction(&cpu, NULL, 0xA9, 0, 0);
  ASSERT_EQ(0x100, cpu.regs.af);
  ASSERT_EQ(2, cpu.regs.bc);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xAF, 0, 0);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  cpu.regs.af = 0x200;
  cpu.regs.hl = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xAC, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);
  ASSERT_EQ(0x100, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, xor_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x300;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0xAE, 0, 0);
  ASSERT_EQ(0x100, cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xAE, 0, 0);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  cpu.regs.af = 0x200;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xAE, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, xor_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x300;
  cpu_run_instruction(&cpu, NULL, 0xEE, 2, 0);
  ASSERT_EQ(0x100, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xEE, 1, 0);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  cpu.regs.af = 0x200;
  cpu_run_instruction(&cpu, NULL, 0xEE, 1, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(6, cpu.regs.pc);
}

UTEST(cpu, or_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x100;
  cpu.regs.bc = 2;
  cpu_run_instruction(&cpu, NULL, 0xB1, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);
  ASSERT_EQ(2, cpu.regs.bc);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, NULL, 0xB7, 0, 0);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  cpu.regs.af = 0x200;
  cpu.regs.hl = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xB4, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);
  ASSERT_EQ(0x100, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, or_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x100;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0xB6, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  cpu.regs.af = 0;
  value = 0;
  cpu_run_instruction(&cpu, &mem, 0xB6, 0, 0);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  cpu.regs.af = 0x200;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xB6, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, or_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xF6, 2, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, NULL, 0xF6, 0, 0);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  cpu.regs.af = 0x200;
  cpu_run_instruction(&cpu, NULL, 0xF6, 1, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(6, cpu.regs.pc);
}

UTEST(cpu, cp_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  cpu.regs.bc = 1;
  cpu_run_instruction(&cpu, NULL, 0xB9, 0, 0);
  ASSERT_EQ(0xF00 | (4 << 4), cpu.regs.af);
  ASSERT_EQ(1, cpu.regs.bc);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xBF, 0, 0);
  ASSERT_EQ(0x100 | (0xC << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu.regs.hl = 0x200;
  cpu_run_instruction(&cpu, NULL, 0xBC, 0, 0);
  ASSERT_EQ(0x100 | (7 << 4), cpu.regs.af);
  ASSERT_EQ(0x200, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, cp_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xBE, 0, 0);
  ASSERT_EQ(0xF00 | (4 << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xBE, 0, 0);
  ASSERT_EQ(0x100 | (0xC << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0xBE, 0, 0);
  ASSERT_EQ(0x100 | (7 << 4), cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, cp_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  cpu_run_instruction(&cpu, NULL, 0xFE, 1, 0);
  ASSERT_EQ(0xF00 | (4 << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xFE, 1, 0);
  ASSERT_EQ(0x100 | (0xC << 4), cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xFE, 2, 0);
  ASSERT_EQ(0x100 | (7 << 4), cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(6, cpu.regs.pc);
}

UTEST(cpu, inc_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 4, 0, 0);
  ASSERT_EQ(0x100, cpu.regs.bc);

  cpu.regs.af = 0xFF00 | (1 << 4);
  cpu_run_instruction(&cpu, NULL, 0x3C, 0, 0);
  ASSERT_EQ(0b1011 << 4, cpu.regs.af);

  ASSERT_EQ((uint32_t)8, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, inc_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, &mem, 0x34, 0, 0);
  ASSERT_EQ(1, mem.memory[0]);
  ASSERT_EQ(0, cpu.regs.af);

  value = 0xFF;
  cpu_run_instruction(&cpu, &mem, 0x34, 0, 0);
  ASSERT_EQ(0, mem.memory[0]);
  ASSERT_EQ(0b1010 << 4, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, dec_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 5, 0, 0);
  ASSERT_EQ(0xFF00, cpu.regs.bc);
  ASSERT_EQ(0b110 << 4, cpu.regs.af);

  cpu.regs.af = 0x100 | (1 << 4);
  cpu_run_instruction(&cpu, NULL, 0x3D, 0, 0);
  ASSERT_EQ(0b1101 << 4, cpu.regs.af);

  ASSERT_EQ((uint32_t)8, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, dec_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, &mem, 0x35, 0, 0);
  ASSERT_EQ(0xFF, mem.memory[0]);
  ASSERT_EQ(0b110 << 4, cpu.regs.af);

  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x35, 0, 0);
  ASSERT_EQ(0, mem.memory[0]);
  ASSERT_EQ(0b1100 << 4, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, ld_r_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 6, 0xde, 0);
  ASSERT_EQ(0xde00, cpu.regs.bc);

  cpu_run_instruction(&cpu, NULL, 0x3E, 0xc0, 0);
  ASSERT_EQ(0xc000, cpu.regs.af);

  ASSERT_EQ((uint32_t)16, cpu.clock);
  ASSERT_EQ(4, cpu.regs.pc);
}

UTEST(cpu, ld_hl_d8)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, &mem, 0x36, 0xff, 0);
  ASSERT_EQ(0xff, mem.memory[0]);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, inc_rr)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 3, 0, 0);
  ASSERT_EQ(1, cpu.regs.bc);

  cpu.regs.sp = 0xFF;
  cpu_run_instruction(&cpu, NULL, 0x33, 0, 0);
  ASSERT_EQ(0x100, cpu.regs.sp);

  ASSERT_EQ((uint32_t)16, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, dec_rr)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 0xB, 0, 0);
  ASSERT_EQ(0xFFFF, cpu.regs.bc);

  cpu.regs.sp = 0x100;
  cpu_run_instruction(&cpu, NULL, 0x3B, 0, 0);
  ASSERT_EQ(0xFF, cpu.regs.sp);

  ASSERT_EQ((uint32_t)16, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, ld_rr_a)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, &mem, 2, 0, 0);
  ASSERT_EQ(0xFF, mem.memory[0]);

  cpu.regs.af = 0xCF00;
  cpu_run_instruction(&cpu, &mem, 0x22, 0, 0);
  ASSERT_EQ(0xCF, mem.memory[0]);
  ASSERT_EQ(1, cpu.regs.hl);

  cpu.regs.af = 0xEF00;
  cpu_run_instruction(&cpu, &mem, 0x32, 0, 0);
  ASSERT_EQ(0xEFCF, (mem.memory[1] << 8) | mem.memory[0]);
  ASSERT_EQ(0, cpu.regs.hl);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, ld_a_rr)
{
  uint16_t value = 0;
  struct memory mem;
  memory_init(&mem, (uint8_t *)&value, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  value = 0xFF;
  cpu_run_instruction(&cpu, &mem, 0xA, 0, 0);
  ASSERT_EQ(0xFF00, cpu.regs.af);

  value = 0xCF;
  cpu_run_instruction(&cpu, &mem, 0x2A, 0, 0);
  ASSERT_EQ(0xCF00, cpu.regs.af);
  ASSERT_EQ(1, cpu.regs.hl);

  value = 0xEF00;
  cpu_run_instruction(&cpu, &mem, 0x3A, 0, 0);
  ASSERT_EQ(0xEF00, cpu.regs.af);
  ASSERT_EQ(0, cpu.regs.hl);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, add_hl_rr)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 0xFF;
  cpu_run_instruction(&cpu, NULL, 9, 0, 0);
  ASSERT_EQ(0xFF, cpu.regs.hl);

  cpu.regs.hl = 0xFFF;
  cpu_run_instruction(&cpu, NULL, 0x29, 0, 0);
  ASSERT_EQ(0x1FFE, cpu.regs.hl);
  ASSERT_EQ(1 << 5, cpu.regs.af);

  cpu.regs.hl = 0xF000;
  cpu_run_instruction(&cpu, NULL, 0x29, 0, 0);
  ASSERT_EQ(0xE000, cpu.regs.hl);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, push_rr)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.sp = 0xD000;
  cpu.regs.bc = 0xDEAD;
  cpu_run_instruction(&cpu, &mem, 0xC5, 0, 0);
  ASSERT_EQ(0xAD, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xDE, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);

  cpu.regs.af = 0xC0FE;
  cpu_run_instruction(&cpu, &mem, 0xF5, 0, 0);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ(0xD000 - 4, cpu.regs.sp);

  ASSERT_EQ((uint32_t)32, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, pop_rr)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  mem.memory[0xD000 - 4] = 0xAD;
  mem.memory[0xD000 - 3] = 0xDE;
  mem.memory[0xD000 - 2] = 0xFE;
  mem.memory[0xD000 - 1] = 0xC0;

  cpu.regs.sp = 0xD000 - 4;
  cpu_run_instruction(&cpu, &mem, 0xD1, 0, 0);
  ASSERT_EQ(0xDEAD, cpu.regs.de);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);

  cpu_run_instruction(&cpu, &mem, 0xE1, 0, 0);
  ASSERT_EQ(0xC0FE, cpu.regs.hl);
  ASSERT_EQ(0xD000, cpu.regs.sp);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, rst)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.sp = 0xD000;
  cpu.regs.pc = 1;
  cpu_run_instruction(&cpu, &mem, 0xDF, 0, 0);
  ASSERT_EQ(0x18, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ(1, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0, mem.memory[cpu.regs.sp + 1]);

  cpu_run_instruction(&cpu, &mem, 0xE7, 0, 0);
  ASSERT_EQ(0x20, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 4, cpu.regs.sp);
  ASSERT_EQ(0x18, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0, mem.memory[cpu.regs.sp + 1]);

  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, jp_nz)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 7;
  cpu_run_instruction(&cpu, NULL, 0xC2, 0, 0);
  ASSERT_EQ(3, cpu.regs.pc);
  ASSERT_EQ((uint32_t)12, cpu.clock);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, NULL, 0xC2, 0xFF, 0xEE);
  ASSERT_EQ(0xEEFF, cpu.regs.pc);
  ASSERT_EQ((uint32_t)28, cpu.clock);
}

UTEST(cpu, jp_nc)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, NULL, 0xD2, 0, 0);
  ASSERT_EQ(3, cpu.regs.pc);
  ASSERT_EQ((uint32_t)12, cpu.clock);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, NULL, 0xD2, 0xFF, 0xEE);
  ASSERT_EQ(0xEEFF, cpu.regs.pc);
  ASSERT_EQ((uint32_t)28, cpu.clock);
}

UTEST(cpu, ld_c_a)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 1;
  cpu.regs.af = 0xE00;
  cpu_run_instruction(&cpu, &mem, 0xE2, 0, 0);
  ASSERT_EQ(0xE, mem.memory[0xFF01]);

  cpu.regs.bc = 2;
  cpu.regs.af = 0xC00;
  cpu_run_instruction(&cpu, &mem, 0xE2, 0, 0);
  ASSERT_EQ(0xC, mem.memory[0xFF02]);

  ASSERT_EQ((uint32_t)16, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, ld_a_c)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  mem.memory[0xFF00] = 0xE;
  cpu_run_instruction(&cpu, &mem, 0xF2, 0, 0);
  ASSERT_EQ(0xE00, cpu.regs.af);

  cpu.regs.bc = 1;
  mem.memory[0xFF01] = 0xC;
  cpu_run_instruction(&cpu, &mem, 0xF2, 0, 0);
  ASSERT_EQ(0xC00, cpu.regs.af);

  ASSERT_EQ((uint32_t)16, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, ret_nz)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.sp = 0xD000 - 2;
  mem.memory[cpu.regs.sp] = 0xAD;
  mem.memory[cpu.regs.sp + 1] = 0xDE;
  cpu_run_instruction(&cpu, &mem, 0xC0, 0, 0);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ((uint32_t)20, cpu.clock);

  cpu.regs.af = 1 << 7;
  cpu_run_instruction(&cpu, &mem, 0xC0, 0, 0);
  ASSERT_EQ(0xDEAD + 1, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ((uint32_t)28, cpu.clock);
}

UTEST(cpu, ret_nc)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.sp = 0xD000 - 2;
  mem.memory[cpu.regs.sp] = 0xAD;
  mem.memory[cpu.regs.sp + 1] = 0xDE;
  cpu_run_instruction(&cpu, &mem, 0xD0, 0, 0);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ((uint32_t)20, cpu.clock);

  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, &mem, 0xD0, 0, 0);
  ASSERT_EQ(0xDEAD + 1, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ((uint32_t)28, cpu.clock);
}

UTEST(cpu, ldh_a8_a)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xE00;
  cpu_run_instruction(&cpu, &mem, 0xE0, 1, 0);
  ASSERT_EQ(0xE, mem.memory[0xFF01]);

  cpu.regs.af = 0xC00;
  cpu_run_instruction(&cpu, &mem, 0xE0, 2, 0);
  ASSERT_EQ(0xC, mem.memory[0xFF02]);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(4, cpu.regs.pc);
}

UTEST(cpu, ldh_a_a8)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  mem.memory[0xFF01] = 0xE;
  cpu_run_instruction(&cpu, &mem, 0xF0, 1, 0);
  ASSERT_EQ(0xE00, cpu.regs.af);

  mem.memory[0xFF02] = 0xC;
  cpu_run_instruction(&cpu, &mem, 0xF0, 2, 0);
  ASSERT_EQ(0xC00, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(4, cpu.regs.pc);
}

UTEST(cpu, jr_nz_r8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 0x20, 0xF, 0);
  ASSERT_EQ(0xF, cpu.regs.pc);
  ASSERT_EQ((uint32_t)12, cpu.clock);

  cpu.regs.af = 1 << 7;
  cpu_run_instruction(&cpu, NULL, 0x20, 0, 0);
  ASSERT_EQ(0x11, cpu.regs.pc);
  ASSERT_EQ((uint32_t) 20, cpu.clock);
}

UTEST(cpu, jr_nc_r8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 0x30, 0xF, 0);
  ASSERT_EQ(0xF, cpu.regs.pc);
  ASSERT_EQ((uint32_t)12, cpu.clock);

  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, NULL, 0x30, 0, 0);
  ASSERT_EQ(0x11, cpu.regs.pc);
  ASSERT_EQ((uint32_t) 20, cpu.clock);
}

UTEST(cpu, jp_a16)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 0xC3, 0xAD, 0xDE);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);

  ASSERT_EQ((uint32_t)16, cpu.clock);
}

UTEST(cpu, call_nz_a16)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.sp = 0xD000;
  cpu.regs.pc = 0xC0FE;
  cpu_run_instruction(&cpu, &mem, 0xC4, 0xAD, 0xDE);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ((uint32_t)24, cpu.clock);

  cpu.regs.af = 1 << 7;
  cpu_run_instruction(&cpu, &mem, 0xC4, 0, 0);
  ASSERT_EQ(0xDEAD + 3, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ((uint32_t)36, cpu.clock);
}

UTEST(cpu, call_nc_a16)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.sp = 0xD000;
  cpu.regs.pc = 0xC0FE;
  cpu_run_instruction(&cpu, &mem, 0xD4, 0xAD, 0xDE);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ((uint32_t)24, cpu.clock);

  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, &mem, 0xD4, 0, 0);
  ASSERT_EQ(0xDEAD + 3, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ((uint32_t)36, cpu.clock);
}

UTEST(cpu, jr_z_r8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 7;
  cpu_run_instruction(&cpu, NULL, 0x28, 0xF, 0);
  ASSERT_EQ(0xF, cpu.regs.pc);
  ASSERT_EQ((uint32_t)12, cpu.clock);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, NULL, 0x28, 0, 0);
  ASSERT_EQ(0x11, cpu.regs.pc);
  ASSERT_EQ((uint32_t) 20, cpu.clock);
}

UTEST(cpu, jr_c_r8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, NULL, 0x38, 0xF, 0);
  ASSERT_EQ(0xF, cpu.regs.pc);
  ASSERT_EQ((uint32_t)12, cpu.clock);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, NULL, 0x38, 0, 0);
  ASSERT_EQ(0x11, cpu.regs.pc);
  ASSERT_EQ((uint32_t) 20, cpu.clock);
}

UTEST(cpu, ret_z)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 7;
  cpu.regs.sp = 0xD000 - 2;
  mem.memory[cpu.regs.sp] = 0xAD;
  mem.memory[cpu.regs.sp + 1] = 0xDE;
  cpu_run_instruction(&cpu, &mem, 0xC8, 0, 0);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ((uint32_t)20, cpu.clock);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, &mem, 0xC8, 0, 0);
  ASSERT_EQ(0xDEAD + 1, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ((uint32_t)28, cpu.clock);
}

UTEST(cpu, ret_c)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 4;
  cpu.regs.sp = 0xD000 - 2;
  mem.memory[cpu.regs.sp] = 0xAD;
  mem.memory[cpu.regs.sp + 1] = 0xDE;
  cpu_run_instruction(&cpu, &mem, 0xD8, 0, 0);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ((uint32_t)20, cpu.clock);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, &mem, 0xD8, 0, 0);
  ASSERT_EQ(0xDEAD + 1, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ((uint32_t)28, cpu.clock);
}
UTEST(cpu, jp_z)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, NULL, 0xCA, 0, 0);
  ASSERT_EQ(3, cpu.regs.pc);
  ASSERT_EQ((uint32_t)12, cpu.clock);

  cpu.regs.af = 1 << 7;
  cpu_run_instruction(&cpu, NULL, 0xCA, 0xFF, 0xEE);
  ASSERT_EQ(0xEEFF, cpu.regs.pc);
  ASSERT_EQ((uint32_t)28, cpu.clock);
}

UTEST(cpu, jp_c)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, NULL, 0xDA, 0, 0);
  ASSERT_EQ(3, cpu.regs.pc);
  ASSERT_EQ((uint32_t)12, cpu.clock);

  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, NULL, 0xDA, 0xFF, 0xEE);
  ASSERT_EQ(0xEEFF, cpu.regs.pc);
  ASSERT_EQ((uint32_t)28, cpu.clock);
}

UTEST(cpu, call_z_a16)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 7;
  cpu.regs.sp = 0xD000;
  cpu.regs.pc = 0xC0FE;
  cpu_run_instruction(&cpu, &mem, 0xCC, 0xAD, 0xDE);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ((uint32_t)24, cpu.clock);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, &mem, 0xCC, 0, 0);
  ASSERT_EQ(0xDEAD + 3, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ((uint32_t)36, cpu.clock);
}

UTEST(cpu, call_c_a16)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 4;
  cpu.regs.sp = 0xD000;
  cpu.regs.pc = 0xC0FE;
  cpu_run_instruction(&cpu, &mem, 0xDC, 0xAD, 0xDE);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ((uint32_t)24, cpu.clock);

  cpu.regs.af = 0;
  cpu_run_instruction(&cpu, &mem, 0xDC, 0, 0);
  ASSERT_EQ(0xDEAD + 3, cpu.regs.pc);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ((uint32_t)36, cpu.clock);
}

UTEST(cpu, add_sp_r8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 0xE8, 1, 0);
  ASSERT_EQ(1, cpu.regs.sp);
  ASSERT_EQ(0, cpu.regs.af);

  cpu_run_instruction(&cpu, NULL, 0xE8, (uint8_t)-1, 0);
  ASSERT_EQ(0, cpu.regs.sp);
  ASSERT_EQ(0b11 << 4, cpu.regs.af);

  cpu_run_instruction(&cpu, NULL, 0xE8, (uint8_t)-1, 0);
  ASSERT_EQ(0xFFFF, cpu.regs.sp);
  ASSERT_EQ(0, cpu.regs.af);

  ASSERT_EQ((uint32_t)48, cpu.clock);
  ASSERT_EQ(6, cpu.regs.pc);
}

UTEST(cpu, ld_hl_sp_r8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 0xF8, 1, 0);
  ASSERT_EQ(1, cpu.regs.hl);
  ASSERT_EQ(0, cpu.regs.af);

  cpu.regs.sp = 1;
  cpu_run_instruction(&cpu, NULL, 0xF8, (uint8_t)-1, 0);
  ASSERT_EQ(0, cpu.regs.hl);
  ASSERT_EQ(0b11 << 4, cpu.regs.af);

  cpu.regs.sp = 0;
  cpu_run_instruction(&cpu, NULL, 0xF8, (uint8_t)-1, 0);
  ASSERT_EQ(0xFFFF, cpu.regs.hl);
  ASSERT_EQ(0, cpu.regs.af);

  ASSERT_EQ((uint32_t)36, cpu.clock);
  ASSERT_EQ(6, cpu.regs.pc);
}

UTEST(cpu, ret)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  mem.memory[0xD000 - 2] = 0xAD;
  mem.memory[0xD000 - 1] = 0xDE;
  cpu.regs.sp = 0xD000 - 2;
  cpu_run_instruction(&cpu, &mem, 0xC9, 0, 0);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ((uint32_t)16, cpu.clock);
}

UTEST(cpu, reti)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  mem.memory[0xD000 - 2] = 0xAD;
  mem.memory[0xD000 - 1] = 0xDE;
  cpu.regs.sp = 0xD000 - 2;
  cpu_run_instruction(&cpu, &mem, 0xD9, 0, 0);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xD000, cpu.regs.sp);
  ASSERT_EQ(1, cpu.interrupts_enabled);
  ASSERT_EQ((uint32_t)16, cpu.clock);
}

UTEST(cpu, jp_hl)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.hl = 0xC0FE;
  cpu_run_instruction(&cpu, NULL, 0xE9, 0, 0);
  ASSERT_EQ(0xC0FE, cpu.regs.pc);
  ASSERT_EQ((uint32_t)4, cpu.clock);
}

UTEST(cpu, ld_sp_hl)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.hl = 0xC0FE;
  cpu_run_instruction(&cpu, NULL, 0xF9, 0, 0);
  ASSERT_EQ(0xC0FE, cpu.regs.sp);
  ASSERT_EQ((uint32_t)8, cpu.clock);
  ASSERT_EQ(1, cpu.regs.pc);
}

UTEST(cpu, ld_a16_a)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, &mem, 0xEA, 0, 0xD0);
  ASSERT_EQ(0xFF, mem.memory[0xD000]);
  ASSERT_EQ((uint32_t)16, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, ld_a_a16)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  mem.memory[0xD000] = 0xFF;
  cpu_run_instruction(&cpu, &mem, 0xFA, 0, 0xD0);
  ASSERT_EQ(0xFF00, cpu.regs.af);
  ASSERT_EQ((uint32_t)16, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, jr_r8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.pc = 2;
  cpu_run_instruction(&cpu, NULL, 0x18, 0xAD, 0);
  ASSERT_EQ(0xAD + 2, cpu.regs.pc);
  ASSERT_EQ((uint32_t)12, cpu.clock);
}

UTEST(cpu, ld_a16_sp)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.sp = 0xC0FE;
  cpu_run_instruction(&cpu, &mem, 8, 0, 0xD0);
  ASSERT_EQ(0xFE, mem.memory[0xD000]);
  ASSERT_EQ(0xC0, mem.memory[0xD001]);

  ASSERT_EQ((uint32_t)20, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, call_a16)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.pc = 0xC0FE;
  cpu.regs.sp = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCD, 0xAD, 0xDE);
  ASSERT_EQ(0xDEAD, cpu.regs.pc);
  ASSERT_EQ(0xFE, mem.memory[cpu.regs.sp]);
  ASSERT_EQ(0xC0, mem.memory[cpu.regs.sp + 1]);
  ASSERT_EQ(0xD000 - 2, cpu.regs.sp);
  ASSERT_EQ((uint32_t)24, cpu.clock);
}

UTEST(cpu, halt)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 0x76, 0, 0);
  ASSERT_TRUE(cpu.halted);
  ASSERT_EQ(1, cpu.regs.pc);
  ASSERT_EQ((uint32_t)4, cpu.clock);
}

UTEST(cpu, di)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.interrupts_enabled = 1;
  cpu_run_instruction(&cpu, NULL, 0xF3, 0, 0);
  ASSERT_FALSE(cpu.interrupts_enabled);
  ASSERT_EQ(1, cpu.regs.pc);
  ASSERT_EQ((uint32_t)4, cpu.clock);
}

UTEST(cpu, ei)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 0xFB, 0, 0);
  ASSERT_TRUE(cpu.interrupts_enabled);
  ASSERT_EQ(1, cpu.regs.pc);
  ASSERT_EQ((uint32_t)4, cpu.clock);
}

UTEST(cpu, rlca)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = (0b111 << 8) | (1 << 4);
  cpu_run_instruction(&cpu, NULL, 7, 0, 0);
  ASSERT_EQ(0b1110 << 8, cpu.regs.af);

  cpu.regs.af = 1 << 15;
  cpu_run_instruction(&cpu, NULL, 7, 0, 0);
  ASSERT_EQ((1 << 4) | (1 << 8), cpu.regs.af);

  ASSERT_EQ(2, cpu.regs.pc);
  ASSERT_EQ((uint32_t)8, cpu.clock);
}

UTEST(cpu, rrca)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = (0b111 << 8) | (1 << 4);
  cpu_run_instruction(&cpu, NULL, 0xF, 0, 0);
  ASSERT_EQ((0b11 << 8) | (1 << 15) | (1 << 4), cpu.regs.af);

  cpu.regs.af = 1 << 15;
  cpu_run_instruction(&cpu, NULL, 0xF, 0, 0);
  ASSERT_EQ((1 << 14), cpu.regs.af);

  ASSERT_EQ(2, cpu.regs.pc);
  ASSERT_EQ((uint32_t)8, cpu.clock);
}

UTEST(cpu, rla)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = (1 << 4) | (1 << 8);
  cpu_run_instruction(&cpu, NULL, 0x17, 0, 0);
  ASSERT_EQ(0b11 << 8, cpu.regs.af);

  cpu.regs.af = 1 << 15;
  cpu_run_instruction(&cpu, NULL, 0x17, 0, 0);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  ASSERT_EQ(2, cpu.regs.pc);
  ASSERT_EQ((uint32_t)8, cpu.clock);
}

UTEST(cpu, rra)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1 << 8;
  cpu_run_instruction(&cpu, NULL, 0x1F, 0, 0);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  cpu.regs.af = (1 << 4) | (1 << 15);
  cpu_run_instruction(&cpu, NULL, 0x1F, 0, 0);
  ASSERT_EQ(0b11 << 14, cpu.regs.af);

  ASSERT_EQ(2, cpu.regs.pc);
  ASSERT_EQ((uint32_t)8, cpu.clock);
}

UTEST(cpu, scf)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0b11100000;
  cpu_run_instruction(&cpu, NULL, 0x37, 0, 0);
  ASSERT_EQ(0b10010000, cpu.regs.af);

  ASSERT_EQ(1, cpu.regs.pc);
  ASSERT_EQ((uint32_t)4, cpu.clock);
}

UTEST(cpu, ccf)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0b11110000;
  cpu_run_instruction(&cpu, NULL, 0x3F, 0, 0);
  ASSERT_EQ(0b1000 << 4, cpu.regs.af);

  cpu.regs.af = 0b11100000;
  cpu_run_instruction(&cpu, NULL, 0x3F, 0, 0);
  ASSERT_EQ(0b10010000, cpu.regs.af);

  ASSERT_EQ(2, cpu.regs.pc);
  ASSERT_EQ((uint32_t)8, cpu.clock);
}

UTEST(cpu, cpl)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xEE00;
  cpu_run_instruction(&cpu, NULL, 0x2F, 0, 0);
  ASSERT_EQ((uint8_t)(~0xEE), (uint8_t)(cpu.regs.af >> 8));
  ASSERT_EQ(0b110 << 4, cpu.regs.af & 0xFF);

  ASSERT_EQ(1, cpu.regs.pc);
  ASSERT_EQ((uint32_t)4, cpu.clock);
}

UTEST(cpu, daa)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x1500;
  cpu_run_instruction(&cpu, NULL, 0xC6, 6, 0);
  cpu_run_instruction(&cpu, NULL, 0x27, 0, 0);
  ASSERT_EQ(0x2100, cpu.regs.af);

  cpu.regs.af = 0x2100;
  cpu_run_instruction(&cpu, NULL, 0xD6, 5, 0);
  cpu_run_instruction(&cpu, NULL, 0x27, 0, 0);
  ASSERT_EQ(0x1600 | (1 << 6), cpu.regs.af);

  cpu.regs.af = 0x9900;
  cpu_run_instruction(&cpu, NULL, 0xC6, 1, 0);
  cpu_run_instruction(&cpu, NULL, 0x27, 0, 0);
  ASSERT_EQ((1 << 7) | (1 << 4), cpu.regs.af);

  ASSERT_EQ(9, cpu.regs.pc);
  ASSERT_EQ((uint32_t)36, cpu.clock);
}

UTEST(cpu, rlc)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 0b11 << 6;
  cpu_run_instruction(&cpu, &mem, 0xCB, 1, 0);
  ASSERT_EQ((1 << 7) | 1, cpu.regs.bc);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  mem.memory[0xD000] = 1 << 7;
  cpu.regs.af = 0;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 6, 0);
  ASSERT_EQ(1, mem.memory[cpu.regs.hl]);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  cpu.regs.hl = 0;
  cpu_run_instruction(&cpu, &mem, 0xCB, 4, 0);
  ASSERT_EQ(0, cpu.regs.hl);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  ASSERT_EQ(6, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, rrc)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 0b11;
  cpu_run_instruction(&cpu, &mem, 0xCB, 9, 0);
  ASSERT_EQ((1 << 7) | 1, cpu.regs.bc);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  mem.memory[0xD000] = 1;
  cpu.regs.af = 0;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0xE, 0);
  ASSERT_EQ(1 << 7, mem.memory[cpu.regs.hl]);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  cpu.regs.hl = 0;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0xC, 0);
  ASSERT_EQ(0, cpu.regs.hl);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  ASSERT_EQ(6, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, rl)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 0b11 << 6;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x11, 0);
  ASSERT_EQ(1 << 7, cpu.regs.bc);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  mem.memory[0xD000] = 1 << 7;
  cpu.regs.af = 0;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x16, 0);
  ASSERT_EQ(0, mem.memory[cpu.regs.hl]);
  ASSERT_EQ((1 << 4) | (1 << 7), cpu.regs.af);

  cpu.regs.hl = 0;
  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x14, 0);
  ASSERT_EQ(1 << 8, cpu.regs.hl);
  ASSERT_EQ(0, cpu.regs.af);

  ASSERT_EQ(6, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, sla)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 0b11 << 6;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x21, 0);
  ASSERT_EQ(1 << 7, cpu.regs.bc);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  mem.memory[0xD000] = 1 << 7;
  cpu.regs.af = 0;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x26, 0);
  ASSERT_EQ(0, mem.memory[cpu.regs.hl]);
  ASSERT_EQ((1 << 4) | (1 << 7), cpu.regs.af);

  cpu.regs.hl = 0;
  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x24, 0);
  ASSERT_EQ(0, cpu.regs.hl);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  ASSERT_EQ(6, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, sra)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = (0b11 << 6) | 1;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x29, 0);
  ASSERT_EQ(0b111 << 5, cpu.regs.bc);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  mem.memory[0xD000] = 1;
  cpu.regs.af = 0;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x2E, 0);
  ASSERT_EQ(0, mem.memory[cpu.regs.hl]);
  ASSERT_EQ((1 << 4) | (1 << 7), cpu.regs.af);

  cpu.regs.hl = 0;
  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x2C, 0);
  ASSERT_EQ(0, cpu.regs.hl);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  ASSERT_EQ(6, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, swap)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 0xAF;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x31, 0);
  ASSERT_EQ(0xFA, cpu.regs.bc);

  mem.memory[0xD000] = 0xFD;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x36, 0);
  ASSERT_EQ(0xDF, mem.memory[cpu.regs.hl]);

  cpu.regs.hl = 0;
  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x34, 0);
  ASSERT_EQ(0, cpu.regs.hl);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  ASSERT_EQ(6, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, srl)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = (0b11 << 6) | 1;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x39, 0);
  ASSERT_EQ(0b11 << 5, cpu.regs.bc);
  ASSERT_EQ(1 << 4, cpu.regs.af);

  mem.memory[0xD000] = 1;
  cpu.regs.af = 0;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x3E, 0);
  ASSERT_EQ(0, mem.memory[cpu.regs.hl]);
  ASSERT_EQ((1 << 4) | (1 << 7), cpu.regs.af);

  cpu.regs.hl = 0;
  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x3C, 0);
  ASSERT_EQ(0, cpu.regs.hl);
  ASSERT_EQ(1 << 7, cpu.regs.af);

  ASSERT_EQ(6, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, bit)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 1;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x41, 0);
  ASSERT_EQ(0b10 << 4, cpu.regs.af);

  mem.memory[0xD000] = 1 << 2;
  cpu.regs.af = 0;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x56, 0);
  ASSERT_EQ(0b10 << 4, cpu.regs.af);

  cpu.regs.hl = (uint8_t)(~(1 << 5)) << 8;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x6C, 0);
  ASSERT_EQ(0b1010 << 4, cpu.regs.af);

  mem.memory[0xD000] = (uint8_t)(~(1 << 7));
  cpu.regs.hl = 0xD000;
  cpu.regs.af = 1 << 4;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x7E, 0);
  ASSERT_EQ(0b1011 << 4, cpu.regs.af);

  ASSERT_EQ(8, cpu.regs.pc);
  ASSERT_EQ((uint32_t)40, cpu.clock);
}

UTEST(cpu, res)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 1;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x81, 0);
  ASSERT_EQ(0, cpu.regs.bc);

  mem.memory[0xD000] = 1 << 2;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0x96, 0);
  ASSERT_EQ(0, mem.memory[cpu.regs.hl]);

  cpu.regs.hl = 0xFF00;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0xAC, 0);
  ASSERT_EQ((uint8_t)(~(1 << 5)) << 8, cpu.regs.hl);

  mem.memory[0xD000] = 0xFF;
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0xBE, 0);
  ASSERT_EQ((uint8_t)(~(1 << 7)), mem.memory[cpu.regs.hl]);

  ASSERT_EQ(8, cpu.regs.pc);
  ASSERT_EQ((uint32_t)48, cpu.clock);
}

UTEST(cpu, set)
{
  struct memory mem;
  memory_init(&mem, NULL, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.bc = 0;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0xC1, 0);
  ASSERT_EQ(1, cpu.regs.bc);

  mem.memory[0xD000] = ~(1 << 2);
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0xD6, 0);
  ASSERT_EQ(0xFF, mem.memory[cpu.regs.hl]);

  cpu.regs.hl = (uint8_t)(~(1 << 5)) << 8;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0xEC, 0);
  ASSERT_EQ(0xFF00, cpu.regs.hl);

  mem.memory[0xD000] = (uint8_t)(~(1 << 7));
  cpu.regs.hl = 0xD000;
  cpu_run_instruction(&cpu, &mem, 0xCB, 0xFE, 0);
  ASSERT_EQ(0xFF, mem.memory[cpu.regs.hl]);

  ASSERT_EQ(8, cpu.regs.pc);
  ASSERT_EQ((uint32_t)48, cpu.clock);
}
