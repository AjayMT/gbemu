
#pragma once

#include <stdint.h>
#include "memory.h"

enum graphics_color
{
  LIGHTER_GREEN = 0, LIGHT_GREEN = 1, DARK_GREEN = 2, DARKER_GREEN = 3
};

struct graphics
{
  enum graphics_color bw_palette[4];
  enum graphics_color obj_0_palette[4];
  enum graphics_color obj_1_palette[4];
};

void graphics_init(struct graphics *graphics);
void graphics_update_color_palette(struct graphics *graphics, uint16_t address, uint8_t value);
