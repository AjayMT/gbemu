
#include <string.h>
#include <stdlib.h>
#include "memory.h"

void memory_init(struct memory *mem, uint8_t *cartridge_data, memory_write_handler_t handler)
{
  memset(mem, 0, sizeof(struct memory));
  mem->cartridge_bank = 1;
  mem->cartridge_data = cartridge_data;
  mem->memory = calloc(0x10000, 1);
  mem->write_handler = handler;
}

uint8_t memory_read_ppu(struct memory *mem, uint16_t addr, uint8_t ppu)
{
  if (addr < 0x4000) return mem->cartridge_data[addr];
  if (addr < 0x8000)
    return mem->cartridge_data[addr + ((mem->cartridge_bank - 1) * 0x4000)];
  if (addr < 0xA000 && !ppu && mem->ppu_mode == pmLCD_MODE_TRANSFER) return 0xFF;
  if (
    addr >= 0xFE00 && addr < 0xFEA0 && !ppu
    && (mem->ppu_mode == pmLCD_MODE_TRANSFER || mem->ppu_mode == pmLCD_MODE_OAM)
    )
    return 0xFF;
  return mem->memory[addr];
}

uint8_t memory_read(struct memory *mem, uint16_t addr)
{
  return memory_read_ppu(mem, addr, 0);
}

void dma_transfer(struct memory *mem, uint8_t value)
{
  mem->memory[ADDR_REG_DMA] = value;
  uint16_t start_address = value << 8;
  for (uint32_t i = 0; i < 0xA0; ++i)
  {
    uint16_t src = start_address + i;
    uint16_t dest = 0xFE00 + i;
    mem->memory[dest] = memory_read_ppu(mem, src, 1);
  }
}

void memory_write(struct memory *mem, uint16_t addr, uint8_t value)
{
  if (addr == ADDR_REG_LCD_Y) mem->memory[addr] = 0;
  else if (addr == ADDR_REG_DIVIDER)
  {
    mem->memory[ADDR_REG_INTERNAL_CLOCK_LOW] = 0;
    mem->memory[addr] = 0;
  }
  else if (addr == ADDR_REG_DMA) dma_transfer(mem, value);
  else if (addr == ADDR_REG_INPUT)
    mem->memory[addr] = (value & 0x30) | (mem->memory[addr] & 0xCF);
  else if (addr == ADDR_REG_LCD_STATUS)
    mem->memory[addr] = (value & 0x7C) | (mem->memory[addr] & 3);
  else if (addr >= ADDR_ROM_BANK_SWITCH_START && addr < ADDR_ROM_BANK_SWITCH_END)
    mem->cartridge_bank = value;
  else mem->memory[addr] = value;

  if (mem->write_handler) mem->write_handler(addr, value);
}
