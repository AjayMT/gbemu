
#include "../src/cpu.h"
#include "utest.h"

UTEST(cpu, ld_rr_nn)
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
