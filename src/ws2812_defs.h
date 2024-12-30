#pragma once
#define WS2812_RESET_US 100
#define WS2812_PIN_BASE 2
#if WS2812_PIN_BASE >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#define LED_MATRIX_WIDTH 16
#define LED_MATRIX_HEIGHT 16
#define LED_MATRICES_PER_STRIP 2
#define LEDS_PER_STRIP (LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT * LED_MATRICES_PER_STRIP) // two 16x16 matrices per strip
#define NMB_STRIPS 3                                                                      // three strips
#if NMB_STRIPS > 8                                                                        // max 8 strips
#error "NMB_STRIPS must be <= 8"
#endif
#if NMB_STRIPS <= 8
typedef uint8_t bit_plane_type; // must be wide enough to contain the number of strips
#endif

#define BYTES_PER_LED 3

// two bit planes, each consists of LEDS_PER_STRIP * BYTES_PER_LED elements of bit_plane_type
// bit planes are effectivley a transposed version of the color values of each led of each strip
// the two bit planes are used for double buffering
static bit_plane_type led_strips_bitplanes[2][LEDS_PER_STRIP * BYTES_PER_LED * 8];
static uint8_t led_colors[NMB_STRIPS * LEDS_PER_STRIP * BYTES_PER_LED]; // color order is GRB (WS2812)
static inline void set_led_color(uint8_t *led, uint8_t r, uint8_t g, uint8_t b)
{
    *led++ = g;
    *led++ = r;
    *led = b;
}
static inline void clear_led_colors()
{
    memset(led_colors, 0, sizeof(led_colors));
}