
#pragma once

#include <stdint.h>

struct input
{
  uint8_t a, b, start, select, up, down, left, right;
  uint8_t direction_flag;
  uint8_t button_flag;
};

void input_init(struct input *input);
uint8_t input_read(struct input *input);
void input_write(struct input *input, uint8_t value);
