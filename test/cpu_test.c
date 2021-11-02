
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
