#pragma once
#include <string.h>

#include "ws2812.hpp"

namespace screen
{
    const auto SCREEN_WIDTH = ws2812::LED_MATRIX_WIDTH * 3;
    const auto SCREEN_HEIGHT = ws2812::LED_MATRIX_HEIGHT * 2;

    extern ws2812::led_color_t scr_screen[SCREEN_HEIGHT][SCREEN_WIDTH] __attribute__((aligned(4)));

    typedef struct screen
    {
        int64_t time_gamma_correction;
        int64_t time_dithering;
        int64_t time_screen_to_led_colors;
        int64_t time_led_colors_to_bitplanes;
        int64_t time_wait_for_DMA;
    } scr_profile_t;

    extern volatile scr_profile_t scr_profile;

    void scr_screen_init();
    void scr_clear_screen();
    void scr_draw_screen(const bool gamma, const bool dithering);
}