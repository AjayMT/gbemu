
#include <stdio.h>
#include <stdlib.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>
#include "memory.h"
#include "cpu.h"
#include "graphics.h"
#include "ppu.h"
#include "timer.h"

#define PIXEL_SIZE 5

struct memory mem;
struct cpu cpu;
struct graphics graphics;
struct ppu ppu;
struct timer timer;

void memory_write_handler(uint16_t addr, uint8_t value)
{
  printf("memory write %hx = %hx\n", addr, value);
  if (addr >= ADDR_BG_MAP_0_START && addr < ADDR_BG_MAP_1_START)
    graphics_update_background_map(&graphics, &mem, ADDR_BG_MAP_0_START);
  if (addr >= ADDR_BG_MAP_1_START && addr < ADDR_BG_MAP_1_END)
    graphics_update_background_map(&graphics, &mem, ADDR_BG_MAP_1_START);
  if (addr >= ADDR_TILE_1_START && addr < ADDR_TILE_0_END)
    graphics_update_tiles(&graphics, &mem, addr);
  if ((addr == ADDR_REG_LCD_CONTROL && value & FLAG_LCD_CONTROL_OBJ_SIZE) || addr == ADDR_REG_DMA)
    graphics_update_sprite_data(&graphics, &mem);
  if (addr == ADDR_REG_BG_PALETTE || addr == ADDR_REG_OB_PALETTE_0 || addr == ADDR_REG_OB_PALETTE_1)
    graphics_update_color_palette(&graphics, addr, value);
}

void boot()
{
  cpu.regs.af = 0x01B0;
  cpu.regs.bc = 0x0013;
  cpu.regs.de = 0x00D8;
  cpu.regs.hl = 0x014D;
  cpu.regs.sp = 0xFFFE;
  cpu.regs.pc = 0x0100;

  memory_write(&mem, 0xFF00, 0xCF);  // gamepad 1100 1111 (no buttons pressed)
  memory_write(&mem, 0xFF05, 0x0);   // TIMA
  memory_write(&mem, 0xFF06, 0x0);   // TMA
  memory_write(&mem, 0xFF07, 0x0);   // TAC
  memory_write(&mem, 0xFF10, 0x80);  // NR10
  memory_write(&mem, 0xFF11, 0xBF);  // NR11
  memory_write(&mem, 0xFF12, 0xF3);  // NR12
  memory_write(&mem, 0xFF14, 0xBF);  // NR14
  memory_write(&mem, 0xFF16, 0x3F);  // NR21
  memory_write(&mem, 0xFF17, 0x00);  // NR22
  memory_write(&mem, 0xFF19, 0xBF);  // NR24
  memory_write(&mem, 0xFF1A, 0x7F);  // NR30
  memory_write(&mem, 0xFF1B, 0xFF);  // NR31
  memory_write(&mem, 0xFF1C, 0x9F);  // NR32
  memory_write(&mem, 0xFF1E, 0xBF);  // NR33
  memory_write(&mem, 0xFF20, 0xFF);  // NR41
  memory_write(&mem, 0xFF21, 0x00);  // NR42
  memory_write(&mem, 0xFF22, 0x00);  // NR43
  memory_write(&mem, 0xFF23, 0xBF);  // NR30
  memory_write(&mem, 0xFF24, 0x77);  // NR50
  memory_write(&mem, 0xFF25, 0xF3);  // NR51
  memory_write(&mem, 0xFF26, 0xF1);  // NR52
  memory_write(&mem, 0xFF40, 0x91);  // LCDC
  memory_write(&mem, 0xFF42, 0x00);  // SCY
  memory_write(&mem, 0xFF43, 0x00);  // SCX
  memory_write(&mem, 0xFF44, 0x00);  // LY
  memory_write(&mem, 0xFF45, 0x00);  // LYC
  memory_write(&mem, 0xFF47, 0xFC);  // BGP
  memory_write(&mem, 0xFF48, 0xFF);  // OBP0
  memory_write(&mem, 0xFF49, 0xFF);  // OBP1
  memory_write(&mem, 0xFF4A, 0x00);  // WY
  memory_write(&mem, 0xFF4B, 0x00);  // WX
  memory_write(&mem, 0xFF0F, 0x00);  // IF
  memory_write(&mem, 0xFFFF, 0x00);  // IE
}

void cycle()
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

  printf("cpu pc: %hx\n", cpu.regs.pc);

  ppu_cycle(&ppu, &mem, &graphics, cycles);
  timer_cycle(&timer, &mem, cycles);
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
  ppu_init(&ppu);
  timer_init(&timer);

  boot();

  sfRenderWindow *window = sfRenderWindow_create(
    (sfVideoMode) { PIXEL_COLUMNS * PIXEL_SIZE, PIXEL_ROWS * PIXEL_SIZE, 32 },
    "GBEMU", sfDefaultStyle, NULL
    );

  sfRectangleShape **pixels = malloc(PIXEL_COLUMNS * PIXEL_ROWS * sizeof(sfRectangleShape *));
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

    sfRenderWindow_clear(window, sfBlack);
    for (uint32_t i = 0; i < PIXEL_COLUMNS * PIXEL_ROWS; ++i)
    {
      sfColor pixel_color;
      switch (ppu.back_buffer[i])
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

  return 0;
}
