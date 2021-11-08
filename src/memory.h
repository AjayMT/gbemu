
#pragma once

#include <stdint.h>

#define ADDR_REG_LCD_Y              0xFF44
#define ADDR_REG_DIVIDER            0xFF04
#define ADDR_REG_INTERNAL_CLOCK_LOW 0xFF03
#define ADDR_REG_DMA                0xFF46
#define ADDR_REG_INPUT              0xFF00
#define ADDR_REG_LCD_STATUS         0xFF41
#define ADDR_ROM_BANK_SWITCH_START  0x2000
#define ADDR_ROM_BANK_SWITCH_END    0x4000

typedef void (*memory_write_handler_t)(uint16_t, uint8_t);

enum ppu_mode {
  pmLCD_MODE_HBLANK = 1,
  pmLCD_MODE_VBLANK = 2,
  pmLCD_MODE_OAM = 2,
  pmLCD_MODE_TRANSFER = 3
};

struct memory {
  uint8_t *cartridge_data;
  uint32_t cartridge_bank;
  uint8_t *memory;
  enum ppu_mode ppu_mode;
  memory_write_handler_t write_handler;
};

void memory_init(struct memory *mem, uint8_t *cartridge_data, memory_write_handler_t handler);
uint8_t memory_read_ppu(struct memory *mem, uint16_t addr, uint8_t ppu);
uint8_t memory_read(struct memory *mem, uint16_t addr);
void memory_write(struct memory *mem, uint16_t addr, uint8_t value);
