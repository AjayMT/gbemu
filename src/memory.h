
#pragma once

#include <stdint.h>

#define ADDR_VECTOR_VBLANK          0x40
#define ADDR_VECTOR_LCD             0x48
#define ADDR_VECTOR_TIMER           0x50
#define ADDR_VECTOR_SERIAL          0x58
#define ADDR_VECTOR_INPUT           0x60
#define ADDR_ROM_BANK_SWITCH_START  0x2000
#define ADDR_ROM_BANK_SWITCH_END    0x4000
#define ADDR_TILE_1_START           0x8000
#define ADDR_TILE_1_END             0x9000
#define ADDR_TILE_0_START           0x8800
#define ADDR_TILE_0_END             0x9800
#define ADDR_BG_MAP_0_START         0x9800
#define ADDR_BG_MAP_1_START         0x9C00
#define ADDR_BG_MAP_1_END           0xA000
#define ADDR_OAM_START              0xFE00
#define ADDR_REG_INPUT              0xFF00
#define ADDR_REG_INTERNAL_CLOCK_LOW 0xFF03
#define ADDR_REG_DIVIDER            0xFF04
#define ADDR_REG_LCD_CONTROL        0xFF40
#define ADDR_REG_LCD_STATUS         0xFF41
#define ADDR_REG_LCD_Y              0xFF44
#define ADDR_REG_DMA                0xFF46
#define ADDR_REG_BG_PALETTE         0xFF47
#define ADDR_REG_OB_PALETTE_0       0xFF48
#define ADDR_REG_OB_PALETTE_1       0xFF49
#define ADDR_REG_INTERRUPT_FLAG     0xFF0F
#define ADDR_REG_INTERRUPT_ENABLED  0xFFFF

typedef void (*memory_write_handler_t)(uint16_t, uint8_t);

enum ppu_mode
{
  pmLCD_MODE_HBLANK = 1,
  pmLCD_MODE_VBLANK = 2,
  pmLCD_MODE_OAM = 2,
  pmLCD_MODE_TRANSFER = 3
};

struct memory
{
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
