
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
  memory_init(&mem, NULL);
  struct cpu cpu;
  cpu_init(&cpu);

  uint8_t expected_byte = memory_read(&mem, cpu.regs.hl);

  cpu.regs.af = ((~expected_byte) << 8) | (uint8_t)(~expected_byte);
  cpu_run_instruction(&cpu, &mem, 0x7E, 0, 0);
  ASSERT_EQ(cpu.regs.af, (uint16_t)((expected_byte << 8) | (uint8_t)(~expected_byte)));

  cpu.regs.bc = ((~expected_byte) << 8) | (uint8_t)(~expected_byte);
  cpu_run_instruction(&cpu, &mem, 0x4E, 0, 0);
  ASSERT_EQ(cpu.regs.bc, (uint16_t)(((~expected_byte) << 8) | expected_byte));

  cpu_run_instruction(&cpu, &mem, 0x46, 0, 0);
  ASSERT_EQ(cpu.regs.bc, (uint16_t)((expected_byte << 8) | expected_byte));

  ASSERT_EQ(cpu.clock, (uint32_t)24);
  ASSERT_EQ(cpu.regs.pc, 3);
}
