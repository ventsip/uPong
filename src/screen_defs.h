#pragma once
#include <string.h>

#include "ws2812_defs.h"

namespace screen
{
    using namespace ws2812;
    const static auto SCREEN_WIDTH = LED_MATRIX_WIDTH * 3;
    const static auto SCREEN_HEIGHT = LED_MATRIX_HEIGHT * 2;
}