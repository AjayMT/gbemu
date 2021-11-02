
#pragma once

#include <stdint.h>

struct memory {

};

uint8_t memory_read(struct memory *mem, uint32_t addr);
uint8_t memory_write(struct memory *mem, uint32_t addr, uint8_t value);
