#pragma once

#include <cstdint>

// Mock version of ws2812.hpp for host testing
#define WS2812_SINGLE

namespace ws2812
{
    const auto LED_MATRIX_WIDTH = 16;
    const auto LED_MATRIX_HEIGHT = 16;
    const auto NMB_STRIP_COLUMNS = 3;
    const auto NMB_STRIP_ROWS = 2;
    const auto NMB_STRIPS = (NMB_STRIP_COLUMNS * NMB_STRIP_ROWS);
    const auto LED_MATRICES_PER_STRIP = 1;
    const auto LEDS_PER_STRIP = (LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT * LED_MATRICES_PER_STRIP);

    typedef struct
    {
        uint8_t padding;
        uint8_t b;
        uint8_t r;
        uint8_t g;
    } led_color_t;

#define ws2812_pack_color(r, g, b) ((ws2812::led_color_t){(uint8_t)(0), (uint8_t)(b), (uint8_t)(r), (uint8_t)(g)})

    // Mock implementations
    extern led_color_t mock_led_colors[NMB_STRIPS][LEDS_PER_STRIP];

    bool WS2812_init();
    void clear_led_colors();
    void transmit_led_colors();
}