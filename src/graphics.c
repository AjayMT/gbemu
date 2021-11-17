
#include <stdio.h>
#include <string.h>
#include "graphics.h"

void graphics_init(struct graphics *graphics)
{
  memset(graphics, 0, sizeof(struct graphics));
}

void graphics_update_color_palette(struct graphics *graphics, uint16_t address, uint8_t value)
{
  enum graphics_color *palette = graphics->bw_palette;
  if (address == ADDR_REG_OB_PALETTE_0)
    palette = graphics->obj_0_palette;
  else if (address == ADDR_REG_OB_PALETTE_1)
    palette = graphics->obj_1_palette;

  palette[0] = (value & 3);
  palette[1] = (value & 0xC) >> 2;
  palette[2] = (value & 0x30) >> 4;
  palette[3] = (value & 0xC0) >> 6;
}
