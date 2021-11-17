
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "graphics.h"
#include "video.h"

void video_init(struct video *video, void (*callback)(uint8_t *))
{
  memset(video, 0, sizeof(struct video));
  video->frame_buffer = calloc(PIXEL_COLUMNS * PIXEL_ROWS, 1);
  video->callback = callback;
}

uint8_t get_pixel_from_line(uint8_t byte1, uint8_t byte2, uint8_t pixel_idx)
{
  uint8_t value_1 = (byte2 >> (7 - pixel_idx)) & 1;
  uint8_t value_2 = (byte1 >> (7 - pixel_idx)) & 1;
  return (uint8_t)((value_1 << 1) | value_2);
}

void draw_bg_line(struct video *video, struct memory *mem, struct graphics *graphics, uint8_t lcd_y)
{
  uint8_t use_tile_set_zero = memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & (1 << 4);
  uint8_t use_tile_map_zero = !(memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & (1 << 3));

  uint16_t tile_set_address = use_tile_set_zero ? ADDR_TILE_0_START : ADDR_TILE_1_START;
  uint16_t tile_map_address = use_tile_map_zero ? ADDR_BG_MAP_0_START : ADDR_BG_MAP_1_START;

  uint32_t screen_y = lcd_y;
  uint32_t scroll_x = memory_read_ppu(mem, ADDR_REG_SCROLL_X, 1);
  uint32_t scroll_y = memory_read_ppu(mem, ADDR_REG_SCROLL_Y, 1);
  for (uint32_t screen_x = 0; screen_x < PIXEL_COLUMNS; ++screen_x)
  {
    uint32_t scrolled_x = screen_x + scroll_x;
    uint32_t scrolled_y = screen_y + scroll_y;

    uint32_t bg_map_x = scrolled_x % 256;
    uint32_t bg_map_y = scrolled_y % 256;

    uint32_t tile_x = bg_map_x / 8;
    uint32_t tile_y = bg_map_y / 8;

    uint32_t tile_pixel_x = bg_map_x % 8;
    uint32_t tile_pixel_y = bg_map_y % 8;

    uint32_t tile_idx = tile_y * 32 + tile_x;
    uint16_t tile_id_address = tile_map_address + tile_idx;

    uint8_t tile_id = memory_read_ppu(mem, tile_id_address, 1);

    uint32_t tile_data_mem_offset = use_tile_set_zero
      ? tile_id * 16
      : (((int8_t)tile_id) + 128) * 16;

    uint32_t tile_data_line_offset = tile_pixel_y * 2;

    uint16_t tile_line_data_start_address = tile_set_address + tile_data_mem_offset
      + tile_data_line_offset;

    uint8_t pixels_1 = memory_read_ppu(mem, tile_line_data_start_address, 1);
    uint8_t pixels_2 = memory_read_ppu(mem, tile_line_data_start_address + 1, 1);

    uint8_t pixel = get_pixel_from_line(pixels_1, pixels_2, tile_pixel_x);
    enum graphics_color color = graphics->bw_palette[pixel];
    video->frame_buffer[screen_y * PIXEL_COLUMNS + screen_x] = (uint8_t)color;
  }
}

void draw_window_line(
  struct video *video, struct memory *mem, struct graphics *graphics, uint8_t lcd_y
  )
{
  uint8_t use_tile_set_zero = memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & (1 << 4);
  uint8_t use_tile_map_zero = !(memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & (1 << 6));

  uint16_t tile_set_address = use_tile_set_zero ? ADDR_TILE_0_START : ADDR_TILE_1_START;
  uint16_t tile_map_address = use_tile_map_zero ? ADDR_BG_MAP_0_START : ADDR_BG_MAP_1_START;

  uint32_t screen_y = lcd_y;
  uint32_t scrolled_y = screen_y - memory_read_ppu(mem, ADDR_REG_WINDOW_Y, 1);

  if (scrolled_y >= PIXEL_ROWS) return;

  for (uint32_t screen_x = 0; screen_x < PIXEL_COLUMNS; ++screen_x)
  {
    uint32_t scrolled_x = screen_x + memory_read_ppu(mem, ADDR_REG_WINDOW_X, 1) - 7;

    uint32_t tile_x = scrolled_x / 8;
    uint32_t tile_y = scrolled_y / 8;

    uint32_t tile_pixel_x = scrolled_x % 8;
    uint32_t tile_pixel_y = scrolled_y % 8;

    uint32_t tile_idx = tile_y * 32 + tile_x;
    uint16_t tile_id_address = tile_map_address + tile_idx;

    uint8_t tile_id = memory_read_ppu(mem, tile_id_address, 1);

    uint32_t tile_data_mem_offset = use_tile_set_zero
      ? tile_id * 16
      : (((int8_t)tile_id) + 128) * 16;

    uint32_t tile_data_line_offset = tile_pixel_y * 2;

    uint16_t tile_line_data_start_address = tile_set_address + tile_data_mem_offset
      + tile_data_line_offset;

    uint8_t pixels_1 = memory_read_ppu(mem, tile_line_data_start_address, 1);
    uint8_t pixels_2 = memory_read_ppu(mem, tile_line_data_start_address + 1, 1);

    uint8_t pixel = get_pixel_from_line(pixels_1, pixels_2, tile_pixel_x);
    enum graphics_color color = graphics->bw_palette[pixel];
    video->frame_buffer[screen_y * PIXEL_COLUMNS + screen_x] = color;
  }
}

void write_scanline(
  struct video *video, struct memory *mem, struct graphics *graphics, uint8_t lcd_y
  )
{
  if (!(memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_LCD_ON)) return;
  if (memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_BG_ON)
    draw_bg_line(video, mem, graphics, lcd_y);
  if (memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_WINDOW_ON)
    draw_window_line(video, mem, graphics, lcd_y);
}

uint8_t *tile_create(struct memory *mem, uint16_t tile_address, uint32_t size_multiplier)
{
  uint8_t *buffer = calloc(size_multiplier * 8 * 8, 1);
  memset(buffer, 0, size_multiplier * 8 * 8);

  for (uint32_t tile_line = 0; tile_line < 8 * size_multiplier; ++tile_line)
  {
    uint32_t index_into_tile = 2 * tile_line;
    uint16_t line_start = tile_address + index_into_tile;

    uint8_t pixels_1 = memory_read_ppu(mem, line_start, 1);
    uint8_t pixels_2 = memory_read_ppu(mem, line_start + 1, 1);

    for (uint8_t i = 0; i < 8; ++i)
      buffer[(tile_line * 8) + i] = get_pixel_from_line(pixels_1, pixels_2, i);
  }

  return buffer;
}

void draw_sprite(
  struct video *video, struct memory *mem, struct graphics *graphics, uint32_t sprite_n
  )
{
  uint16_t offset_in_oam = sprite_n * 4;
  uint16_t oam_start = 0xFE00 + offset_in_oam;
  uint8_t sprite_y = memory_read_ppu(mem, oam_start, 1);
  uint8_t sprite_x = memory_read_ppu(mem, oam_start + 1, 1);

  if (sprite_y == 0 || sprite_y >= 160) return;
  if (sprite_x == 0 || sprite_x >= 168) return;

  uint8_t sprite_size_multiplier = memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & (1 << 2)
    ? 2 : 1;

  uint16_t tile_set_location = ADDR_TILE_0_START;

  uint8_t pattern_n = memory_read_ppu(mem, oam_start + 2, 1);
  uint8_t sprite_attrs = memory_read_ppu(mem, oam_start + 3, 1);

  uint8_t use_palette_1 = sprite_attrs & (1 << 4);
  uint8_t flip_x = sprite_attrs & (1 << 5);
  uint8_t flip_y = sprite_attrs & (1 << 6);
  uint8_t obj_behind_bg = sprite_attrs & (1 << 7);

  enum graphics_color *palette = use_palette_1 ? graphics->obj_1_palette : graphics->obj_0_palette;

  uint32_t tile_offset = pattern_n * 16;

  uint16_t pattern_address = tile_set_location + tile_offset;
  uint8_t *tile = tile_create(mem, pattern_address, sprite_size_multiplier);
  int32_t start_y = sprite_y - 16;
  int32_t start_x = sprite_x - 8;

  for (uint32_t y = 0; y < 8 * sprite_size_multiplier; ++y)
  {
    for (uint32_t x = 0; x < 8; ++x)
    {
      uint32_t maybe_flipped_y = !flip_y ? y : (8 * sprite_size_multiplier) - y - 1;
      uint32_t maybe_flipped_x = !flip_x ? x : 8 - x - 1;

      uint8_t pixel = tile[maybe_flipped_y * 8 + maybe_flipped_x];
      if (pixel == 0) continue;

      int32_t screen_x = start_x + x;
      int32_t screen_y = start_y + y;

      if (screen_x >= PIXEL_COLUMNS || screen_y >= PIXEL_ROWS) continue;

      uint8_t existing_pixel = video->frame_buffer[screen_y * PIXEL_COLUMNS + screen_x];
      if (obj_behind_bg && existing_pixel != LIGHTER_GREEN) continue;

      video->frame_buffer[screen_y * PIXEL_COLUMNS + screen_x] = palette[pixel];
    }
  }

  free(tile);
}

void video_cycle(
  struct video *video, struct memory *mem, struct graphics *graphics, uint32_t cycles
  )
{
  video->cycles += cycles;

  switch (mem->ppu_mode)
  {
  case pmLCD_MODE_OAM:
  {
    if (video->cycles < 80) break;
    video->cycles %= 80;
    memory_set_and_write_ppu_mode(mem, pmLCD_MODE_TRANSFER);
    break;
  }

  case pmLCD_MODE_TRANSFER:
  {
    if (video->cycles < 172) break;
    video->cycles %= 172;

    memory_set_and_write_ppu_mode(mem, pmLCD_MODE_HBLANK);

    if (memory_read_ppu(mem, ADDR_REG_LCD_STATUS, 1) & FLAG_LCD_STATUS_HBLANK_INTERRUPT_ON)
      mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_LCD;

    uint8_t ly_coincidence_interrupt = memory_read_ppu(mem, ADDR_REG_LCD_STATUS, 1)
      & FLAG_LCD_STATUS_LCDYC_INTERRUPT_ON;
    uint8_t lcd_y_compare = memory_read_ppu(mem, ADDR_REG_LCD_Y_COMPARE, 1);
    uint8_t lcd_y = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1);
    if (ly_coincidence_interrupt && lcd_y_compare == lcd_y)
      mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_LCD;

    if (lcd_y_compare == lcd_y) mem->memory[ADDR_REG_LCD_STATUS] |= FLAG_LCD_STATUS_COINCIDENCE;
    else mem->memory[ADDR_REG_LCD_STATUS] &= ~FLAG_LCD_STATUS_COINCIDENCE;

    break;
  }

  case pmLCD_MODE_HBLANK:
  {
    if (video->cycles < 204) break;
    video->cycles %= 204;

    uint8_t lcd_y = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1);
    write_scanline(video, mem, graphics, lcd_y);
    lcd_y++;
    mem->memory[ADDR_REG_LCD_Y] = lcd_y;

    if (lcd_y == 144)
    {
      memory_set_and_write_ppu_mode(mem, pmLCD_MODE_VBLANK);
      mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_VBLANK;
    }
    else memory_set_and_write_ppu_mode(mem, pmLCD_MODE_OAM);

    break;
  }

  case pmLCD_MODE_VBLANK:
  {
    if (video->cycles < 456) break;
    video->cycles %= 456;

    uint8_t lcd_y = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1) + 1;
    mem->memory[ADDR_REG_LCD_Y] = lcd_y;

    if (lcd_y == 154)
    {
      if (memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_OBJ_ON)
        for (uint32_t i = 0; i < 40; ++i)
          draw_sprite(video, mem, graphics, i);
      video->callback(video->frame_buffer);
      memset(video->frame_buffer, LIGHTER_GREEN, PIXEL_COLUMNS * PIXEL_ROWS);
      mem->memory[ADDR_REG_LCD_Y] = 0;
      memory_set_and_write_ppu_mode(mem, pmLCD_MODE_OAM);
    }

    break;
  }
  }
}
