
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppu.h"

void ppu_init(struct ppu *ppu)
{
  ppu->cycles = 0;
  memset(ppu->front_sprite_buffer, TRANSPARENT, sizeof(ppu->front_sprite_buffer));
  memset(ppu->back_sprite_buffer, TRANSPARENT, sizeof(ppu->back_sprite_buffer));
  memset(ppu->front_bg_buffer, LIGHTER_GREEN, sizeof(ppu->front_bg_buffer));
  memset(ppu->back_bg_buffer, LIGHTER_GREEN, sizeof(ppu->back_bg_buffer));
}

void oam(struct ppu *ppu, struct memory *mem)
{
  if (ppu->cycles < 80) return;

  if (memory_read_ppu(mem, ADDR_REG_LCD_STATUS, 1) & FLAG_LCD_STATUS_OAM_INTERRUPT_ON)
    mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_LCD;

  uint8_t lcd_y = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1);
  uint8_t lcd_y_compare = memory_read_ppu(mem, ADDR_REG_LCD_Y_COMPARE, 1);
  if (lcd_y == lcd_y_compare)
  {
    mem->memory[ADDR_REG_LCD_STATUS] |= FLAG_LCD_STATUS_COINCIDENCE;
    if (memory_read_ppu(mem, ADDR_REG_LCD_STATUS, 1) & FLAG_LCD_STATUS_LCDYC_INTERRUPT_ON)
      mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_LCD;
  }
  else mem->memory[ADDR_REG_LCD_STATUS] &= ~FLAG_LCD_STATUS_COINCIDENCE;

  memory_set_and_write_ppu_mode(mem, pmLCD_MODE_TRANSFER);
  ppu->cycles -= 80;
}

void transfer(struct ppu *ppu, struct memory *mem)
{
  if (ppu->cycles < 173) return;
  memory_set_and_write_ppu_mode(mem, pmLCD_MODE_HBLANK);
  ppu->cycles -= 173;
}

int compare_sprites(const void *va, const void *vb)
{
  const struct graphics_sprite *a = va;
  const struct graphics_sprite *b = vb;
  return a->x < b->x || (a->x == b->x && a->address < b->address);
}

void draw_sprites(struct ppu *ppu, struct memory *mem, struct graphics *graphics)
{
  if (!(memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_OBJ_ON)) return;

  uint8_t lcd_y = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1);
  uint8_t count = 0;
  struct graphics_sprite sprites[40];
  for (uint32_t i = 0; i < 40; ++i)
  {
    struct graphics_sprite sprite = graphics->sprites[i];
    if (
      lcd_y >= sprite.y - 16
      && ((sprite.big && lcd_y < sprite.y) || (!sprite.big && lcd_y < sprite.y - 8))
      )
    {
      sprites[count] = sprite;
      ++count;
    }
  }

  qsort(sprites, 40, sizeof(struct graphics_sprite), compare_sprites);
  uint8_t start_idx = count < 10 ? count : 10;
  for (int32_t i = start_idx - 1; i >= 0; --i)
  {
    struct graphics_sprite sprite = sprites[i];
    uint8_t *tile = graphics->tile_1[sprite.tile_idx];
    uint8_t tile_y_correction = 16;
    if (sprite.big && lcd_y >= sprite.y - 8 && !sprite.flip_y)
    {
      tile = graphics->tile_1[sprite.tile_idx + 1];
      tile_y_correction = 8;
    }

    uint8_t tile_y = lcd_y - (sprite.y - tile_y_correction);
    if (sprite.flip_y) tile_y = abs(lcd_y - (sprite.y - tile_y_correction + 7));

    for (int32_t j = 0; j < 8; ++j)
    {
      uint8_t tile_x = sprite.flip_x ? (7 - j) : j;
      uint8_t pixel = tile[tile_y * 8 + tile_x];
      enum graphics_color color = graphics->obj_0_palette[pixel];
      if (sprite.palette_1) color = graphics->obj_1_palette[pixel];

      int16_t buf_x = sprite.x - 8 + j;
      if (
        !(sprite.bg_prio && ppu->back_bg_buffer[lcd_y * PIXEL_COLUMNS + buf_x] != LIGHTER_GREEN)
        && color != TRANSPARENT && buf_x < 160 && buf_x >= 0
        )
        ppu->back_sprite_buffer[lcd_y * PIXEL_COLUMNS + buf_x] = (uint8_t)color;
    }
  }
}

void draw_background(struct ppu *ppu, struct memory *mem, struct graphics *graphics)
{
  if (!(memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_BG_ON)) return;

  uint8_t lcd_y = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1);
  uint8_t scroll_y = memory_read_ppu(mem, ADDR_REG_SCROLL_Y, 1);
  uint8_t scroll_x = memory_read_ppu(mem, ADDR_REG_SCROLL_X, 1);

  uint8_t start_background_tile_y = (scroll_y + lcd_y) / 8;
  if (start_background_tile_y >= 32) start_background_tile_y %= 32;
  uint8_t background_tile_y_offset = (scroll_y + lcd_y) % 8;

  uint8_t start_background_tile_x = scroll_x / 8;
  uint8_t background_tile_x_offset = scroll_x % 8;

  uint8_t *bg_map = graphics_get_background_map(graphics, mem, FLAG_LCD_CONTROL_BG_MAP);
  for (int32_t i = 0; i < 21; ++i)
  {
    uint8_t tile_idx_x = start_background_tile_x + i;
    if (tile_idx_x >= 32) tile_idx_x %= 32;
    uint8_t *tile = graphics_get_background_tile(
      graphics, mem, bg_map[start_background_tile_y * 32 + tile_idx_x]
      );

    for (int32_t j = 0; j < 8; ++j)
    {
      if (i == 0 && j < background_tile_x_offset) continue;
      if (i == 20 && j >= background_tile_x_offset) continue;

      uint8_t pixel = tile[background_tile_y_offset * 8 + j];
      enum graphics_color color = graphics->bw_palette[pixel];
      int32_t buf_x = (i * 8) + j - background_tile_x_offset;
      ppu->back_bg_buffer[lcd_y * PIXEL_COLUMNS + buf_x] = (uint8_t)color;
    }
  }
}

void draw_window(struct ppu *ppu, struct memory *mem, struct graphics *graphics)
{
  if (!(memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_WINDOW_ON)) return;

  uint8_t lcd_y = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1);
  int16_t window_y = memory_read_ppu(mem, ADDR_REG_WINDOW_Y, 1);
  int16_t window_x = memory_read_ppu(mem, ADDR_REG_WINDOW_X, 1);

  if (lcd_y < window_y) return;

  uint8_t stop_off_screen = 0;
  uint8_t *window_map = graphics_get_background_map(graphics, mem, FLAG_LCD_CONTROL_WINDOW_MAP);
  for (int32_t tile_x = 0; tile_x < 32; ++tile_x)
  {
    uint8_t tile_idx = window_map[((lcd_y - window_y) / 8) * 32 + tile_x];
    uint8_t *tile = graphics->tile_0[(int8_t)tile_idx + 128];
    for (int32_t j = 0; j < 8; ++j)
    {
      if ((tile_x * 8) + j + window_x > 159)
      {
        stop_off_screen = 1;
        break;
      }

      uint8_t pixel = tile[((lcd_y - window_y) % 8) * 8 + j];
      enum graphics_color color = graphics->bw_palette[pixel];
      ppu->back_bg_buffer[lcd_y * PIXEL_COLUMNS + (tile_x * 8) + j + window_x] = (uint8_t)color;
    }
    if (stop_off_screen) break;
  }
}

void hblank(struct ppu *ppu, struct memory *mem, struct graphics *graphics)
{
  if (ppu->cycles < 204) return;

  if (memory_read_ppu(mem, ADDR_REG_LCD_STATUS, 1) & FLAG_LCD_STATUS_HBLANK_INTERRUPT_ON)
    mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_LCD;

  draw_background(ppu, mem, graphics);
  draw_window(ppu, mem, graphics);
  draw_sprites(ppu, mem, graphics);

  if (memory_read_ppu(mem, ADDR_REG_LCD_Y, 1) == 143)
    memory_set_and_write_ppu_mode(mem, pmLCD_MODE_VBLANK);
  else memory_set_and_write_ppu_mode(mem, pmLCD_MODE_OAM);

  mem->memory[ADDR_REG_LCD_Y] = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1) + 1;

  ppu->cycles -= 204;
}

void vblank(struct ppu *ppu, struct memory *mem)
{
  if (ppu->cycles < 456) return;

  if (memory_read_ppu(mem, ADDR_REG_LCD_Y, 1) == 144)
  {
    mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_VBLANK;
    memcpy(ppu->front_sprite_buffer, ppu->back_sprite_buffer, sizeof(ppu->front_sprite_buffer));
    memcpy(ppu->front_bg_buffer, ppu->back_bg_buffer, sizeof(ppu->front_bg_buffer));
    memset(ppu->back_sprite_buffer, TRANSPARENT, sizeof(ppu->back_sprite_buffer));
    memset(ppu->back_bg_buffer, LIGHTER_GREEN, sizeof(ppu->back_bg_buffer));
  }

  if (memory_read_ppu(mem, ADDR_REG_LCD_Y, 1) == 153)
  {
    mem->memory[ADDR_REG_LCD_Y] = 0;
    memory_set_and_write_ppu_mode(mem, pmLCD_MODE_OAM);
  }
  else mem->memory[ADDR_REG_LCD_Y] = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1) + 1;

  ppu->cycles -= 456;
}

void ppu_cycle(struct ppu *ppu, struct memory *mem, struct graphics *graphics, uint32_t cycles)
{
  if (!(memory_read_ppu(mem, ADDR_REG_LCD_CONTROL, 1) & FLAG_LCD_CONTROL_LCD_ON)) return;

  ppu->cycles += cycles;
  switch (mem->ppu_mode)
  {
  case pmLCD_MODE_OAM:
    oam(ppu, mem); break;
  case pmLCD_MODE_TRANSFER:
    transfer(ppu, mem); break;
  case pmLCD_MODE_HBLANK:
    hblank(ppu, mem, graphics); break;
  case pmLCD_MODE_VBLANK:
    vblank(ppu, mem); break;
  }
}
