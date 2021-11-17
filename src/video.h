
#pragma once

#include "memory.h"

#define PIXEL_COLUMNS 160
#define PIXEL_ROWS    144

enum video_color
{
  WHITE = 0, LIGHT_GRAY = 1, DARK_GRAY = 2, BLACK = 3
};

struct video
{
  uint32_t cycles;
  uint8_t *frame_buffer;
  void (*callback)(uint8_t *);
};

void video_init(struct video *video, void (*callback)(uint8_t *));
void video_cycle(struct video *video, struct memory *mem, uint32_t cycles);
