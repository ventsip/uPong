#pragma once
#include <string.h>

#include "ws2812.hpp"

namespace screen
{
    const auto SCREEN_WIDTH = ws2812::LED_MATRIX_WIDTH * 3;
    const auto SCREEN_HEIGHT = ws2812::LED_MATRIX_HEIGHT * 2;

    extern ws2812::led_color_t scr_screen[SCREEN_HEIGHT][SCREEN_WIDTH] __attribute__((aligned(4)));

    void screen_init();
    void clear_screen();
    void screen_to_led_colors(const bool gamma_correction, const bool dithering);
}