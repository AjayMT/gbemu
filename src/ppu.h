
#pragma once

#include "memory.h"
#include "graphics.h"

#define PIXEL_COLUMNS 160
#define PIXEL_ROWS    144

struct ppu
{
  uint8_t front_sprite_buffer[PIXEL_COLUMNS * PIXEL_ROWS];
  uint8_t back_sprite_buffer[PIXEL_COLUMNS * PIXEL_ROWS];
  uint8_t front_bg_buffer[PIXEL_COLUMNS * PIXEL_ROWS];
  uint8_t back_bg_buffer[PIXEL_COLUMNS * PIXEL_ROWS];
  uint32_t cycles;
};

void ppu_init(struct ppu *ppu);
void ppu_cycle(struct ppu *ppu, struct memory *mem, struct graphics *graphics, uint32_t cycles);
