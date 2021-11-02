
#include "../src/cpu.h"
#include "utest.h"

UTEST(cpu, ld_bc_d16)
{
  struct cpu cpu;
  cpu_init(&cpu);
  cpu_run_instruction(&cpu, NULL, 0x01, 0xad, 0xde);
  ASSERT_EQ(cpu.regs.bc, 0xdead);
}
