
#include <stdio.h>
#include <SFML/Window.h>
#include <SFML/Graphics.h>

int main(int argc, char *argv[])
{
  // to suppress compiler warning about unused arguments
  (void)argc;
  (void)argv;

  sfColor background_color = sfColor_fromRGB(15, 56, 15);
  sfRenderWindow *window = sfRenderWindow_create(
    (sfVideoMode) { 800, 600, 32 }, "GBEMU", sfDefaultStyle, NULL
    );

  while (sfRenderWindow_isOpen(window))
  {
    sfEvent event;
    while (sfRenderWindow_pollEvent(window, &event))
    {
      if (event.type == sfEvtClosed)
        sfRenderWindow_close(window);
    }

    sfRenderWindow_clear(window, background_color);
    sfRenderWindow_display(window);
  }

  return 0;
}
