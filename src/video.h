
#pragma once

#include "graphics.h"
#include "memory.h"

#define PIXEL_COLUMNS 160
#define PIXEL_ROWS    144

struct video
{
  uint32_t cycles;
  uint8_t *frame_buffer;
  void (*callback)(uint8_t *);
};

void video_init(struct video *video, void (*callback)(uint8_t *));
void video_cycle(
  struct video *video, struct memory *mem, struct graphics *graphics, uint32_t cycles
  );
