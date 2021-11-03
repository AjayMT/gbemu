
#include <string.h>
#include "memory.h"

void memory_init(struct memory *mem, uint8_t *cartridge_data)
{
  memset(mem, 0, sizeof(struct memory));
  mem->cartridge_data = cartridge_data;
}

uint8_t memory_read(struct memory *mem, uint16_t addr)
{
  if (addr < 0x4000) return mem->cartridge_data[addr];

  return 0;
}

void memory_write(struct memory *mem, uint16_t addr, uint8_t value)
{
  (void)mem;
  (void)addr;
  (void)value;
}
