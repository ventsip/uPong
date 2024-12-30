#pragma once
#define WS2812_RESET_US 100
#define WS2812_PIN_BASE 2
#if WS2812_PIN_BASE >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#define LEDS_PER_STRIP (16 * 16 * 2)
#define NMB_STRIPS 3 // max 8 strips
#if NMB_STRIPS > 8
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