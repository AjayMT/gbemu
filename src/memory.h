
#pragma once

#include <stdint.h>

struct memory {
  uint8_t *cartridge_data;        // 0000 - 3FFF is the fixed bank, 4000 - 7FFF are switchable banks
  uint8_t vram[0x2000];           // 8000 - 9FFF
  uint8_t external_ram[0x2000];   // A000 - BFFF
  uint8_t wram[0x2000];           // C000 - DFFF
  uint8_t oam[0xA0];              // FE00 - FE9F
  uint8_t ioreg[0x80];            // FF00 - FF7F
  uint8_t hram[0x7f];             // FF80 - FFFE
  uint8_t ie;                     // FFFF
};

// E000 - FDFF is an echo of C000 - DDFF
// FEA0 - FEFF is not usable

void memory_init(struct memory *mem, uint8_t *cartridge_data);
uint8_t memory_read(struct memory *mem, uint16_t addr);
void memory_write(struct memory *mem, uint16_t addr, uint8_t value);
