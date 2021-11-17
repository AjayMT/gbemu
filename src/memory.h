
#pragma once

#include <stdint.h>

#define ADDR_VECTOR_VBLANK          0x40
#define ADDR_VECTOR_LCD             0x48
#define ADDR_VECTOR_TIMER           0x50
#define ADDR_VECTOR_SERIAL          0x58
#define ADDR_VECTOR_INPUT           0x60
#define ADDR_ROM_BANK_SWITCH_START  0x2000
#define ADDR_ROM_BANK_SWITCH_END    0x4000
#define ADDR_TILE_0_START           0x8000
#define ADDR_TILE_0_END             0x9000
#define ADDR_TILE_1_START           0x8800
#define ADDR_TILE_1_END             0x9800
#define ADDR_BG_MAP_0_START         0x9800
#define ADDR_BG_MAP_1_START         0x9C00
#define ADDR_BG_MAP_1_END           0xA000
#define ADDR_OAM_START              0xFE00
#define ADDR_REG_INPUT              0xFF00
#define ADDR_REG_INTERNAL_CLOCK_LOW 0xFF03
#define ADDR_REG_DIVIDER            0xFF04
#define ADDR_REG_TIMA               0xFF05
#define ADDR_REG_TMA                0xFF06
#define ADDR_REG_TAC                0xFF07
#define ADDR_REG_LCD_CONTROL        0xFF40
#define ADDR_REG_LCD_STATUS         0xFF41
#define ADDR_REG_SCROLL_Y           0xFF42
#define ADDR_REG_SCROLL_X           0xFF43
#define ADDR_REG_LCD_Y              0xFF44
#define ADDR_REG_LCD_Y_COMPARE      0xFF45
#define ADDR_REG_DMA                0xFF46
#define ADDR_REG_BG_PALETTE         0xFF47
#define ADDR_REG_OB_PALETTE_0       0xFF48
#define ADDR_REG_OB_PALETTE_1       0xFF49
#define ADDR_REG_WINDOW_Y           0xFF4A
#define ADDR_REG_WINDOW_X           0xFF4B
#define ADDR_REG_INTERRUPT_FLAG     0xFF0F
#define ADDR_REG_INTERRUPT_ENABLED  0xFFFF

#define FLAG_INTERRUPT_VBLANK               1
#define FLAG_INTERRUPT_LCD                  2
#define FLAG_INTERRUPT_TIMER                4
#define FLAG_INTERRUPT_SERIAL               8
#define FLAG_INTERRUPT_INPUT                16
#define FLAG_LCD_CONTROL_LCD_ON             128
#define FLAG_LCD_CONTROL_BG_ON              1
#define FLAG_LCD_CONTROL_OBJ_ON             2
#define FLAG_LCD_CONTROL_OBJ_SIZE           4
#define FLAG_LCD_CONTROL_WINDOW_ON          32
#define FLAG_LCD_CONTROL_WINDOW_MAP         64
#define FLAG_LCD_CONTROL_BG_MAP             8
#define FLAG_LCD_CONTROL_BG_DATA            16
#define FLAG_LCD_STATUS_HBLANK_INTERRUPT_ON 8
#define FLAG_LCD_STATUS_OAM_INTERRUPT_ON    32
#define FLAG_LCD_STATUS_LCDYC_INTERRUPT_ON  64
#define FLAG_LCD_STATUS_COINCIDENCE         4
#define FLAG_LCD_STATUS_MODE_HIGH           2
#define FLAG_LCD_STATUS_MODE_LOW            1
#define FLAG_TIMER_CLOCK_MODE               3
#define FLAG_TIMER_START                    4

typedef void (*memory_write_handler_t)(uint16_t, uint8_t);

enum ppu_mode
{
  pmLCD_MODE_HBLANK = 0,
  pmLCD_MODE_VBLANK = 1,
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
void memory_set_and_write_ppu_mode(struct memory *mem, enum ppu_mode mode);
