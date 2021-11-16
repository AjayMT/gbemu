
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppu.h"

void ppu_init(struct ppu *ppu)
{
  ppu->cycles = 0;
  memset(ppu->front_buffer, LIGHTER_GREEN, sizeof(ppu->front_buffer));
  memset(ppu->back_buffer, LIGHTER_GREEN, sizeof(ppu->back_buffer));
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

  printf("drawing sprites, count %u\n", count);

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

    for (uint32_t j = 0; j < 8; ++j)
    {
      uint8_t tile_x = sprite.flip_x ? (7 - j) : j;
      uint8_t pixel = tile[tile_y * 8 + tile_x];
      enum graphics_color color = graphics->obj_0_palette[pixel];
      if (sprite.palette_1) color = graphics->obj_1_palette[pixel];

      int16_t buf_x = sprite.x - 8 + j;
      // TODO bg prio stuff
      printf("pixel %d %d color %d\n", buf_x, lcd_y, color);
      ppu->back_buffer[lcd_y * PIXEL_COLUMNS + buf_x] = (uint8_t)color;
    }
  }
}

void draw_background(struct ppu *ppu, struct memory *mem, struct graphics *graphics)
{
  (void)ppu;
  (void)mem;
  (void)graphics;
}

void hblank(struct ppu *ppu, struct memory *mem, struct graphics *graphics)
{
  if (ppu->cycles < 204) return;

  if (memory_read_ppu(mem, ADDR_REG_LCD_STATUS, 1) & FLAG_LCD_STATUS_HBLANK_INTERRUPT_ON)
    mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_LCD;

  draw_background(ppu, mem, graphics);
  draw_sprites(ppu, mem, graphics);

  if (memory_read_ppu(mem, ADDR_REG_LCD_Y, 1) == 143)
    memory_set_and_write_ppu_mode(mem, pmLCD_MODE_VBLANK);
  else memory_set_and_write_ppu_mode(mem, pmLCD_MODE_OAM);

  mem->memory[ADDR_REG_LCD_Y] = memory_read_ppu(mem, ADDR_REG_LCD_Y, 1) + 1;

  ppu->cycles -= 204;
}

void vblank(struct ppu *ppu, struct memory *mem)
{
  printf("vblank\n");
  if (ppu->cycles < 456) return;

  if (memory_read_ppu(mem, ADDR_REG_LCD_Y, 1) == 144)
  {
    mem->memory[ADDR_REG_INTERRUPT_FLAG] |= FLAG_INTERRUPT_VBLANK;
    memcpy(ppu->front_buffer, ppu->back_buffer, sizeof(ppu->front_buffer));
    memset(ppu->back_buffer, LIGHTER_GREEN, sizeof(ppu->back_buffer));
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
