
#include <stdio.h>
#include <stdlib.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>
#include "cpu.h"

#define PIXEL_COLUMNS 160
#define PIXEL_ROWS    144
#define PIXEL_SIZE    5

int main(int argc, char *argv[])
{
  // to suppress compiler warning about unused arguments
  (void)argc;
  (void)argv;

  sfRenderWindow *window = sfRenderWindow_create(
    (sfVideoMode) { PIXEL_COLUMNS * PIXEL_SIZE, PIXEL_ROWS * PIXEL_SIZE, 32 },
    "GBEMU", sfDefaultStyle, NULL
    );

  sfColor background_color = sfColor_fromRGB(15, 56, 15);
  sfColor foreground_color = sfColor_fromRGB(155, 188, 15);
  sfRectangleShape **pixels = malloc(PIXEL_COLUMNS * PIXEL_ROWS * sizeof(sfRectangleShape *));
  for (uint32_t i = 0; i < PIXEL_COLUMNS * PIXEL_ROWS; ++i)
  {
    pixels[i] = sfRectangleShape_create();
    if (((i % PIXEL_COLUMNS) + (i / PIXEL_COLUMNS)) % 2 == 0)
      sfRectangleShape_setFillColor(pixels[i], foreground_color);
    else
      sfRectangleShape_setFillColor(pixels[i], background_color);
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

    sfRenderWindow_clear(window, background_color);
    for (uint32_t i = 0; i < PIXEL_COLUMNS * PIXEL_ROWS; ++i)
    {
      sfRenderWindow_drawRectangleShape(window, pixels[i], NULL);
    }
    sfRenderWindow_display(window);
  }

  return 0;
}
