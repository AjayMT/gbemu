
#pragma once

#include <stdint.h>
#include "memory.h"

enum graphics_color
{
  LIGHTER_GREEN = 0, LIGHT_GREEN = 1, DARK_GREEN = 2, DARKER_GREEN = 3, TRANSPARENT = 4
};

struct graphics_sprite
{
  uint16_t address;
  uint8_t y;
  uint8_t x;
  uint8_t tile_idx;
  uint8_t palette_1;
  uint8_t flip_x;
  uint8_t flip_y;
  uint8_t bg_prio;
  uint8_t big;
};

struct graphics
{
  struct graphics_sprite sprites[40];
  uint8_t *background_map_0;
  uint8_t *background_map_1;
  uint8_t *tile_0[256];
  uint8_t *tile_1[256];
  enum graphics_color bw_palette[4];
  enum graphics_color obj_0_palette[4];
  enum graphics_color obj_1_palette[4];
};

void graphics_init(struct graphics *graphics);
void graphics_update_sprite_data(struct graphics *graphics, struct memory *mem);
uint8_t *graphics_get_background_map(struct graphics *graphics, struct memory *mem, uint8_t flag);
void graphics_update_background_map(
  struct graphics *graphics, struct memory *mem, uint16_t start_address
  );
void graphics_update_tiles(struct graphics *graphics, struct memory *mem, uint16_t address);
uint8_t *graphics_get_background_tile(
  struct graphics *graphics, struct memory *mem, uint8_t tile_idx
  );
void graphics_update_color_palette(struct graphics *graphics, uint16_t address, uint8_t value);
