
#pragma once

#include <stdint.h>

struct memory {

};

void memory_init(struct memory *mem);
uint8_t memory_read(struct memory *mem, uint16_t addr);
void memory_write(struct memory *mem, uint16_t addr, uint8_t value);
