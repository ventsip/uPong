#pragma once

#define WS2812_SINGLE

namespace ws2812
{
    const auto LED_MATRIX_WIDTH = 16;
    const auto LED_MATRIX_HEIGHT = 16;
    const auto NMB_STRIPS = 3;
    const auto LED_MATRICES_PER_STRIP = 2;
    const auto LEDS_PER_STRIP = (LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT * LED_MATRICES_PER_STRIP); // two 16x16 matrices per strip

#ifdef WS2812_PARALLEL
#pragma pack(push, 1)
    typedef struct
    {
        uint8_t g;
        uint8_t r;
        uint8_t b;
        uint8_t padding;
    } led_color_t;
#pragma pack(pop)
#define ws2812_pack_color(r, g, b) ((led_color_t){(uint8_t)(g), (uint8_t)(r), (uint8_t)(b), 0})
#endif
#ifdef WS2812_SINGLE
    typedef struct
    {
        uint8_t padding;
        uint8_t b;
        uint8_t r;
        uint8_t g;
    } led_color_t;
#define ws2812_pack_color(r, g, b) ((ws2812::led_color_t){(uint8_t)(0), (uint8_t)(b), (uint8_t)(r), (uint8_t)(g)})
#endif

#ifdef WS2812_PARALLEL
    extern led_color_t led_colors[NMB_STRIPS][LEDS_PER_STRIP] __attribute__((aligned(4)));
#endif
#ifdef WS2812_SINGLE
    extern led_color_t (*led_colors)[NMB_STRIPS][LEDS_PER_STRIP];
#endif

    bool WS2812_init();
    void clear_led_colors();
    void wait_for_led_colors_transmission();
    void transmit_led_colors();
}
