#pragma once

#include <stdint.h>
#include <stdbool.h>

void decode(uint8_t *datain, size_t x_in, size_t y_in, size_t frame, size_t frameall, uint8_t *dataout);
