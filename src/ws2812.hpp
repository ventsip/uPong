#pragma once

#include <cstdint>

#define WS2812_SINGLE

namespace ws2812
{
    const auto LED_MATRIX_WIDTH = 16;
    const auto LED_MATRIX_HEIGHT = 16;
    const auto NMB_STRIPS = 3;
    const auto LED_MATRICES_PER_STRIP = 2;
    const auto LEDS_PER_STRIP = (LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT * LED_MATRICES_PER_STRIP); // two 16x16 matrices per strip

#ifdef WS2812_PARALLEL
#if NMB_STRIPS > 8 // max 8 strips
#error "NMB_STRIPS must be <= 8"
#endif
#if NMB_STRIPS <= 8
    typedef uint8_t bit_plane_t; // must be wide enough to contain the number of strips
#endif
    const auto BITS_PER_COLOR_COMPONENT = 8;
    const auto BYTES_PER_WS2812_LED = 3;
    typedef struct
    {
        bit_plane_t led[BYTES_PER_WS2812_LED][BITS_PER_COLOR_COMPONENT];
    } led_bit_planes_t;
#endif

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
#define ws2812_pack_color(r, g, b) ((ws2812::led_color_t){(uint8_t)(g), (uint8_t)(r), (uint8_t)(b), 0})
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
    extern led_bit_planes_t led_strips_bitstream[2][LEDS_PER_STRIP] __attribute__((aligned(4)));
    extern led_color_t led_colors[NMB_STRIPS][LEDS_PER_STRIP] __attribute__((aligned(4)));
#endif
#ifdef WS2812_SINGLE
    extern led_color_t (*led_colors)[NMB_STRIPS][LEDS_PER_STRIP];
#endif

    bool WS2812_init();
    void clear_led_colors();
    void wait_for_led_colors_transmission();
#ifdef WS2812_SINGLE
    void transmit_led_colors();
#endif

#ifdef WS2812_PARALLEL
    void transmit_led_colors_dma(int active_planes);
    void led_colors_to_bitplanes_standard(
        led_bit_planes_t *const bitplane,
        const led_color_t *const colors);

    void led_colors_to_bitplanes(
        led_bit_planes_t *const bitplane,
        const led_color_t *const colors);
#endif
}
