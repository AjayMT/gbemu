
#include <string.h>
#include "input.h"

void input_init(struct input *input)
{
  memset(input, 0, sizeof(struct input));
}

uint8_t input_read(struct input *input)
{
  uint8_t buttons = 0b1111;

  if (input->direction_flag)
  {
    buttons = input->right ? (buttons & (~1)) : (buttons | 1);
    buttons = input->left ? (buttons & (~2)) : (buttons | 2);
    buttons = input->up ? (buttons & (~4)) : (buttons | 4);
    buttons = input->down ? (buttons & (~8)) : (buttons | 8);
  }

  if (input->button_flag)
  {
    buttons = input->a ? (buttons & (~1)) : (buttons | 1);
    buttons = input->b ? (buttons & (~2)) : (buttons | 2);
    buttons = input->select ? (buttons & (~4)) : (buttons | 4);
    buttons = input->start ? (buttons & (~8)) : (buttons | 8);
  }

  buttons = input->direction_flag ? (buttons & (~16)) : (buttons | 16);
  buttons = input->button_flag ? (buttons & (~32)) : (buttons | 32);

  return buttons;
}

void input_write(struct input *input, uint8_t value)
{
  input->direction_flag = !(value & 16);
  input->button_flag = !(value & 32);
}
