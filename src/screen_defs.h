#pragma once
#include <string.h>

#include "ws2812_defs.h"

#define SCREEN_WIDTH (LED_MATRIX_WIDTH * 3)
#define SCREEN_HEIGHT (LED_MATRIX_HEIGHT * 2)
#define BYTES_PER_PIXEL 3 // 3 bytes per pixel, color order is GRB, following the WS2812 standard
#if BYTES_PER_PIXEL != BYTES_PER_LED
#error "BYTES_PER_PIXEL must be equal to BYTES_PER_LED"
#endif