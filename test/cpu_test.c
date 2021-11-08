
#include "../src/cpu.h"
#include "utest.h"

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
  memory_init(&mem, &zero);
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
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, &mem, 0x77, 0, 0);
  ASSERT_EQ(value, 0xFF);

  cpu.regs.bc = 0xde;
  cpu_run_instruction(&cpu, &mem, 0x71, 0, 0);
  ASSERT_EQ(0xde, value);

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
  ASSERT_EQ(0xE001, cpu.regs.af);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, NULL, 0x87, 0, 0);
  ASSERT_EQ(0xFE03, cpu.regs.af);

  cpu.regs.af = 0xF000;
  cpu.regs.hl = 0x1000;
  cpu_run_instruction(&cpu, NULL, 0x84, 0, 0);
  ASSERT_EQ(9, cpu.regs.af);
  ASSERT_EQ(0x1000, cpu.regs.hl);

  ASSERT_EQ(4, cpu.regs.pc);
  ASSERT_EQ((uint32_t)16, cpu.clock);
}

UTEST(cpu, add_a_hl)
{
  struct memory mem;
  uint8_t value = 0;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0;
  value = 3;
  cpu_run_instruction(&cpu, &mem, 0x86, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  cpu.regs.af = 0xF000;
  value = 0xF0;
  cpu_run_instruction(&cpu, &mem, 0x86, 0, 0);
  ASSERT_EQ(0xE001, cpu.regs.af);

  cpu.regs.af = 0xFF00;
  value = 0xFF;
  cpu_run_instruction(&cpu, &mem, 0x86, 0, 0);
  ASSERT_EQ(0xFE03, cpu.regs.af);

  cpu.regs.af = 0xF000;
  value = 0x10;
  cpu_run_instruction(&cpu, &mem, 0x86, 0, 0);
  ASSERT_EQ(9, cpu.regs.af);

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
  ASSERT_EQ(0xE001, cpu.regs.af);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, NULL, 0xC6, 0xFF, 0);
  ASSERT_EQ(0xFE03, cpu.regs.af);

  cpu.regs.af = 0xF000;
  cpu_run_instruction(&cpu, NULL, 0xC6, 0x10, 0);
  ASSERT_EQ(9, cpu.regs.af);

  ASSERT_EQ(8, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, adc_a_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x601;
  cpu.regs.bc = 3;
  cpu_run_instruction(&cpu, NULL, 0x89, 0, 0);
  ASSERT_EQ(0xa00, cpu.regs.af);

  cpu.regs.af = 0xF001;
  cpu_run_instruction(&cpu, NULL, 0x8F, 0, 0);
  ASSERT_EQ(0xE101, cpu.regs.af);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, NULL, 0x8F, 0, 0);
  ASSERT_EQ(0xFE03, cpu.regs.af);

  cpu.regs.af = 0xF001;
  cpu.regs.hl = 0xF00;
  cpu_run_instruction(&cpu, NULL, 0x8C, 0, 0);
  ASSERT_EQ(11, cpu.regs.af);
  ASSERT_EQ(0xF00, cpu.regs.hl);

  ASSERT_EQ(4, cpu.regs.pc);
  ASSERT_EQ((uint32_t)16, cpu.clock);
}

UTEST(cpu, adc_a_hl)
{
  struct memory mem;
  uint8_t value = 0;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1;
  value = 3;
  cpu_run_instruction(&cpu, &mem, 0x8E, 0, 0);
  ASSERT_EQ(0x400, cpu.regs.af);

  cpu.regs.af = 0xF000;
  value = 0xF0;
  cpu_run_instruction(&cpu, &mem, 0x8E, 0, 0);
  ASSERT_EQ(0xE001, cpu.regs.af);

  cpu.regs.af = 0xFF01;
  value = 0xFF;
  cpu_run_instruction(&cpu, &mem, 0x8E, 0, 0);
  ASSERT_EQ(0xFF03, cpu.regs.af);

  cpu.regs.af = 0xF001;
  value = 0xF;
  cpu_run_instruction(&cpu, &mem, 0x8E, 0, 0);
  ASSERT_EQ(11, cpu.regs.af);

  ASSERT_EQ(4, cpu.regs.pc);
  ASSERT_EQ((uint32_t)32, cpu.clock);
}

UTEST(cpu, adc_a_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 1;
  cpu_run_instruction(&cpu, NULL, 0xCE, 3, 0);
  ASSERT_EQ(0x400, cpu.regs.af);

  cpu.regs.af = 0xF000;
  cpu_run_instruction(&cpu, NULL, 0xCE, 0xF0, 0);
  ASSERT_EQ(0xE001, cpu.regs.af);

  cpu.regs.af = 0xFF01;
  cpu_run_instruction(&cpu, NULL, 0xCE, 0xFF, 0);
  ASSERT_EQ(0xFF03, cpu.regs.af);

  cpu.regs.af = 0xF001;
  cpu_run_instruction(&cpu, NULL, 0xCE, 0xF, 0);
  ASSERT_EQ(11, cpu.regs.af);

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
  ASSERT_EQ(0xE04, cpu.regs.af);
  ASSERT_EQ(1, cpu.regs.bc);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0x97, 0, 0);
  ASSERT_EQ(0xC, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu.regs.hl = 0x200;
  cpu_run_instruction(&cpu, NULL, 0x94, 0, 0);
  ASSERT_EQ(0xFF07, cpu.regs.af);
  ASSERT_EQ(0x200, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, sub_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x96, 0, 0);
  ASSERT_EQ(0xE04, cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x96, 0, 0);
  ASSERT_EQ(0xC, cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0x96, 0, 0);
  ASSERT_EQ(0xFF07, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, sub_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  cpu_run_instruction(&cpu, NULL, 0xD6, 1, 0);
  ASSERT_EQ(0xE04, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xD6, 1, 0);
  ASSERT_EQ(0xC, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xD6, 2, 0);
  ASSERT_EQ(0xFF07, cpu.regs.af);

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
  ASSERT_EQ(0xE04, cpu.regs.af);
  ASSERT_EQ(1, cpu.regs.bc);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0x9F, 0, 0);
  ASSERT_EQ(0xC, cpu.regs.af);

  cpu.regs.af = 0x101;
  cpu.regs.hl = 0x100;
  cpu_run_instruction(&cpu, NULL, 0x9C, 0, 0);
  ASSERT_EQ(0xFF07, cpu.regs.af);
  ASSERT_EQ(0x100, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, sbc_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x9E, 0, 0);
  ASSERT_EQ(0xE04, cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x9E, 0, 0);
  ASSERT_EQ(0xC, cpu.regs.af);

  cpu.regs.af = 0x101;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x9E, 0, 0);
  ASSERT_EQ(0xFF07, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, sbc_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  cpu_run_instruction(&cpu, NULL, 0xDE, 1, 0);
  ASSERT_EQ(0xE04, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xDE, 1, 0);
  ASSERT_EQ(0xC, cpu.regs.af);

  cpu.regs.af = 0x101;
  cpu_run_instruction(&cpu, NULL, 0xDE, 1, 0);
  ASSERT_EQ(0xFF07, cpu.regs.af);

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
  ASSERT_EQ(0x202, cpu.regs.af);
  ASSERT_EQ(2, cpu.regs.bc);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xA7, 0, 0);
  ASSERT_EQ(0x102, cpu.regs.af);

  cpu.regs.af = 0x200;
  cpu.regs.hl = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xA4, 0, 0);
  ASSERT_EQ(0b1010, cpu.regs.af);
  ASSERT_EQ(0x100, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, and_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x300;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0xA6, 0, 0);
  ASSERT_EQ(0x202, cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xA6, 0, 0);
  ASSERT_EQ(0x102, cpu.regs.af);

  cpu.regs.af = 0x200;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xA6, 0, 0);
  ASSERT_EQ(0b1010, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, and_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x300;
  cpu_run_instruction(&cpu, NULL, 0xE6, 2, 0);
  ASSERT_EQ(0x202, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xE6, 1, 0);
  ASSERT_EQ(0x102, cpu.regs.af);

  cpu.regs.af = 0x200;
  cpu_run_instruction(&cpu, NULL, 0xE6, 1, 0);
  ASSERT_EQ(0b1010, cpu.regs.af);

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
  ASSERT_EQ(0b1000, cpu.regs.af);

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
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x300;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0xAE, 0, 0);
  ASSERT_EQ(0x100, cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xAE, 0, 0);
  ASSERT_EQ(0b1000, cpu.regs.af);

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
  ASSERT_EQ(0b1000, cpu.regs.af);

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
  ASSERT_EQ(0b1000, cpu.regs.af);

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
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0x100;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0xB6, 0, 0);
  ASSERT_EQ(0x300, cpu.regs.af);

  cpu.regs.af = 0;
  value = 0;
  cpu_run_instruction(&cpu, &mem, 0xB6, 0, 0);
  ASSERT_EQ(0b1000, cpu.regs.af);

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
  ASSERT_EQ(0b1000, cpu.regs.af);

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
  ASSERT_EQ(0xF04, cpu.regs.af);
  ASSERT_EQ(1, cpu.regs.bc);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xBF, 0, 0);
  ASSERT_EQ(0x10C, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu.regs.hl = 0x200;
  cpu_run_instruction(&cpu, NULL, 0xBC, 0, 0);
  ASSERT_EQ(0x107, cpu.regs.af);
  ASSERT_EQ(0x200, cpu.regs.hl);

  ASSERT_EQ((uint32_t)12, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, cp_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xBE, 0, 0);
  ASSERT_EQ(0xF04, cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 1;
  cpu_run_instruction(&cpu, &mem, 0xBE, 0, 0);
  ASSERT_EQ(0x10C, cpu.regs.af);

  cpu.regs.af = 0x100;
  value = 2;
  cpu_run_instruction(&cpu, &mem, 0xBE, 0, 0);
  ASSERT_EQ(0x107, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, cp_d8)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xF00;
  cpu_run_instruction(&cpu, NULL, 0xFE, 1, 0);
  ASSERT_EQ(0xF04, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xFE, 1, 0);
  ASSERT_EQ(0x10C, cpu.regs.af);

  cpu.regs.af = 0x100;
  cpu_run_instruction(&cpu, NULL, 0xFE, 2, 0);
  ASSERT_EQ(0x107, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(6, cpu.regs.pc);
}

UTEST(cpu, inc_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 4, 0, 0);
  ASSERT_EQ(0x100, cpu.regs.bc);

  cpu.regs.af = 0xFF01;
  cpu_run_instruction(&cpu, NULL, 0x3C, 0, 0);
  ASSERT_EQ(0b1011, cpu.regs.af);

  ASSERT_EQ((uint32_t)8, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, inc_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, &mem, 0x34, 0, 0);
  ASSERT_EQ(1, value);
  ASSERT_EQ(0, cpu.regs.af);

  value = 0xFF;
  cpu_run_instruction(&cpu, &mem, 0x34, 0, 0);
  ASSERT_EQ(0, value);
  ASSERT_EQ(0b1010, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, dec_r)
{
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, NULL, 5, 0, 0);
  ASSERT_EQ(0xFF00, cpu.regs.bc);
  ASSERT_EQ(0b110, cpu.regs.af);

  cpu.regs.af = 0x101;
  cpu_run_instruction(&cpu, NULL, 0x3D, 0, 0);
  ASSERT_EQ(0b1101, cpu.regs.af);

  ASSERT_EQ((uint32_t)8, cpu.clock);
  ASSERT_EQ(2, cpu.regs.pc);
}

UTEST(cpu, dec_hl)
{
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, &mem, 0x35, 0, 0);
  ASSERT_EQ(0xFF, value);
  ASSERT_EQ(0b110, cpu.regs.af);

  value = 1;
  cpu_run_instruction(&cpu, &mem, 0x35, 0, 0);
  ASSERT_EQ(0, value);
  ASSERT_EQ(0b1100, cpu.regs.af);

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
  uint8_t value = 0;
  struct memory mem;
  memory_init(&mem, &value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu_run_instruction(&cpu, &mem, 0x36, 0xff, 0);
  ASSERT_EQ(0xff, value);

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
  uint16_t value = 0;
  struct memory mem;
  memory_init(&mem, (uint8_t *)&value);
  struct cpu cpu;
  cpu_init(&cpu);

  cpu.regs.af = 0xFF00;
  cpu_run_instruction(&cpu, &mem, 2, 0, 0);
  ASSERT_EQ(0xFF, value);

  cpu.regs.af = 0xCF00;
  cpu_run_instruction(&cpu, &mem, 0x22, 0, 0);
  ASSERT_EQ(0xCF, value);
  ASSERT_EQ(1, cpu.regs.hl);

  cpu.regs.af = 0xEF00;
  cpu_run_instruction(&cpu, &mem, 0x32, 0, 0);
  ASSERT_EQ(0xEFCF, value);
  ASSERT_EQ(0, cpu.regs.hl);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}

UTEST(cpu, ld_a_rr)
{
  uint16_t value = 0;
  struct memory mem;
  memory_init(&mem, (uint8_t *)&value);
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
  ASSERT_EQ(0b10, cpu.regs.af);

  cpu.regs.hl = 0xF000;
  cpu_run_instruction(&cpu, NULL, 0x29, 0, 0);
  ASSERT_EQ(0xE000, cpu.regs.hl);
  ASSERT_EQ(1, cpu.regs.af);

  ASSERT_EQ((uint32_t)24, cpu.clock);
  ASSERT_EQ(3, cpu.regs.pc);
}
