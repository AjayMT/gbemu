
#include <stdio.h>
#include <stdlib.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>
#include "memory.h"
#include "cpu.h"
#include "graphics.h"
#include "video.h"
#include "timer.h"

#define PIXEL_SIZE 5

struct memory mem;
struct cpu cpu;
struct graphics graphics;
struct video video;
struct timer timer;
sfRenderWindow *window;
sfRectangleShape **pixels;

void memory_write_handler(uint16_t addr, uint8_t value)
{
  if (addr == ADDR_REG_BG_PALETTE || addr == ADDR_REG_OB_PALETTE_0 || addr == ADDR_REG_OB_PALETTE_1)
    graphics_update_color_palette(&graphics, addr, value);
}

void cycle()
{
  int32_t remaining_cycles = 70368;
  while (remaining_cycles > 0)
  {
    uint32_t cycles = 0;
    cpu_handle_interrupts(&cpu, &mem);
    if (!cpu.halted)
    {
      uint8_t a = memory_read(&mem, cpu.regs.pc);
      uint8_t b = memory_read(&mem, cpu.regs.pc + 1);
      uint8_t c = memory_read(&mem, cpu.regs.pc + 2);
      uint32_t clock_initial = cpu.clock;
      cpu_run_instruction(&cpu, &mem, a, b, c);
      cycles = cpu.clock - clock_initial;
    }
    else cycles = 4;

    video_cycle(&video, &mem, &graphics, cycles);
    timer_cycle(&timer, &mem, cycles);

    remaining_cycles -= cycles;
  }
}

void draw_callback(uint8_t *frame_buffer)
{
  sfRenderWindow_clear(window, sfBlack);
  for (uint32_t i = 0; i < PIXEL_COLUMNS * PIXEL_ROWS; ++i)
  {
    sfColor pixel_color;
    switch (frame_buffer[i])
    {
    case LIGHTER_GREEN:
      pixel_color = sfColor_fromRGB(155, 188, 15);
      break;
    case LIGHT_GREEN:
      pixel_color = sfColor_fromRGB(139, 172, 15);
      break;
    case DARK_GREEN:
      pixel_color = sfColor_fromRGB(48, 98, 48);
      break;
    case DARKER_GREEN:
      pixel_color = sfColor_fromRGB(15, 56, 15);
      break;
    }

    sfRectangleShape_setFillColor(pixels[i], pixel_color);
    sfRenderWindow_drawRectangleShape(window, pixels[i], NULL);
  }
  sfRenderWindow_display(window);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Usage: gbemu <filename>\n");
    return 1;
  }

  FILE *rom = fopen(argv[1], "r");
  fseek(rom, 0, SEEK_END);
  off_t len = ftello(rom);
  fseek(rom, 0, SEEK_SET);
  uint8_t *rom_data = malloc(len);
  fread(rom_data, 1, len, rom);

  memory_init(&mem, rom_data, memory_write_handler);
  cpu_init(&cpu);
  graphics_init(&graphics);
  video_init(&video, draw_callback);
  timer_init(&timer);

  window = sfRenderWindow_create(
    (sfVideoMode) { PIXEL_COLUMNS * PIXEL_SIZE, PIXEL_ROWS * PIXEL_SIZE, 32 },
    "GBEMU", sfDefaultStyle, NULL
    );

  pixels = malloc(PIXEL_COLUMNS * PIXEL_ROWS * sizeof(sfRectangleShape *));
  for (uint32_t i = 0; i < PIXEL_COLUMNS * PIXEL_ROWS; ++i)
  {
    pixels[i] = sfRectangleShape_create();
    sfRectangleShape_setFillColor(pixels[i], sfWhite);
    sfRectangleShape_setSize(pixels[i], (sfVector2f) { PIXEL_SIZE, PIXEL_SIZE });
    sfRectangleShape_setPosition(
      pixels[i], (sfVector2f) { (i % PIXEL_COLUMNS) * PIXEL_SIZE, (i / PIXEL_COLUMNS) * PIXEL_SIZE }
      );
  }

  while (sfRenderWindow_isOpen(window))
  {
    sfEvent event;
    while (sfRenderWindow_pollEvent(window, &event))
    {
      if (event.type == sfEvtClosed)
        sfRenderWindow_close(window);
    }

    cycle();
  }

  return 0;
}
