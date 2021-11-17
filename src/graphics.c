
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"

void graphics_init(struct graphics *graphics)
{
  memset(graphics, 0, sizeof(struct graphics));
  graphics->background_map_0 = malloc(32 * 32);
  graphics->background_map_1 = malloc(32 * 32);
  for (uint32_t i = 0; i < 256; ++i)
  {
    graphics->tile_0[i] = malloc(8 * 8);
    graphics->tile_1[i] = malloc(8 * 8);
  }
}

void graphics_update_sprite_data(struct graphics *graphics, struct memory *mem)
{
  uint8_t big_sprite = memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_OBJ_SIZE;
  for (uint32_t i = 0; i < 40; ++i)
  {
    uint8_t y = memory_read_ppu(mem, ADDR_OAM_START + (i * 4), 1);
    uint8_t x = memory_read_ppu(mem, ADDR_OAM_START + (i * 4) + 1, 1);
    uint8_t tile_idx = memory_read_ppu(mem, ADDR_OAM_START + (i * 4) + 2, 1);
    uint8_t attributes = memory_read_ppu(mem, ADDR_OAM_START + (i * 4) + 3, 1);

    struct graphics_sprite sprite;
    sprite.address = ADDR_OAM_START + (i * 4);
    sprite.y = y;
    sprite.x = x;
    sprite.big = big_sprite != 0;
    sprite.tile_idx = big_sprite ? tile_idx & 0xFE : tile_idx;
    sprite.flip_x = attributes & 32;
    sprite.flip_y = attributes & 64;
    sprite.bg_prio = attributes & 128;
    sprite.palette_1 = attributes & 16;
    if (sprite.flip_y && big_sprite) sprite.tile_idx++;

    graphics->sprites[i] = sprite;
  }
}

uint8_t *graphics_get_background_map(struct graphics *graphics, struct memory *mem, uint8_t flag)
{
  uint8_t map_1 = memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & flag;
  if (map_1) return graphics->background_map_1;
  return graphics->background_map_0;
}

void graphics_update_background_map(
  struct graphics *graphics, struct memory *mem, uint16_t start_address
  )
{
  uint8_t *map = graphics->background_map_0;
  if (start_address == ADDR_BG_MAP_1_START) map = graphics->background_map_1;
  for (uint32_t i = 0; i < 32 * 32; ++i)
    map[i] = memory_read_ppu(mem, start_address + i, 1);
}

void update_tile(uint8_t **tiles, struct memory *mem, uint16_t start_address)
{
  for (uint32_t i = 0; i < 256; ++i)
  {
    uint16_t address = start_address + (i * 16);
    for (uint32_t j = 0; j < 8; ++j)
    {
      uint8_t line_data_1 = memory_read_ppu(mem, address + (j * 2), 1);
      uint8_t line_data_2 = memory_read_ppu(mem, address + (j * 2) + 1, 1);
      for (int32_t k = 7; k >= 0; --k)
      {
        uint8_t low = (line_data_1 >> k) & 1;
        uint8_t high = (line_data_2 >> k) & 1;
        tiles[i][(j * 8) + (abs(k - 7) % 8)] = (high << 1) | low;
      }
    }
  }
}

void graphics_update_tiles(struct graphics *graphics, struct memory *mem, uint16_t address)
{
  if (address >= ADDR_TILE_0_START && address < ADDR_TILE_0_END)
    update_tile(graphics->tile_0, mem, ADDR_TILE_0_START);
  if (address >= ADDR_TILE_1_START && address < ADDR_TILE_1_END)
    update_tile(graphics->tile_1, mem, ADDR_TILE_1_START);
}

uint8_t *graphics_get_background_tile(
  struct graphics *graphics, struct memory *mem, uint8_t tile_idx
  )
{
  uint8_t flag = memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_BG_DATA;
  if (flag) return graphics->tile_1[tile_idx];
  return graphics->tile_0[(uint8_t)(tile_idx + 128)];
}

void graphics_update_color_palette(struct graphics *graphics, uint16_t address, uint8_t value)
{
  enum graphics_color *palette = graphics->bw_palette;
  uint8_t lower_is_transparent = 0;
  if (address == ADDR_REG_OB_PALETTE_0)
  {
    palette = graphics->obj_0_palette;
    lower_is_transparent = 1;
  }
  else if (address == ADDR_REG_OB_PALETTE_1)
  {
    palette = graphics->obj_1_palette;
    lower_is_transparent = 1;
  }

  palette[0] = lower_is_transparent ? TRANSPARENT : (value & 3);
  palette[1] = (value & 0xC) >> 2;
  palette[2] = (value & 0x30) >> 4;
  palette[3] = (value & 0xC0) >> 6;
}
