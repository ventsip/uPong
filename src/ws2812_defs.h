#pragma once
#ifdef WS2812_PARALLEL
#define WS2812_RESET_US 100
#endif
#ifdef WS2812_SINGLE
#define WS2812_RESET_US 300
#endif
#define WS2812_PIN_BASE 2
#if WS2812_PIN_BASE >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#define LED_MATRIX_WIDTH 16
#define LED_MATRIX_HEIGHT 16
#define LED_MATRICES_PER_STRIP 2
#define LEDS_PER_STRIP (LED_MATRIX_WIDTH * LED_MATRIX_HEIGHT * LED_MATRICES_PER_STRIP) // two 16x16 matrices per strip
#define NMB_STRIPS 3                                                                   // three strips
#if NMB_STRIPS > 8                                                                     // max 8 strips
#error "NMB_STRIPS must be <= 8"
#endif

#ifdef WS2812_PARALLEL
#if NMB_STRIPS <= 8
typedef uint8_t bit_plane_t; // must be wide enough to contain the number of strips
#endif
#define BITS_PER_COLOR_COMPONENT 8
#define BYTES_PER_WS2812_LED 3
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
#define ws2812_pack_color(r, g, b) ((led_color_t){(uint8_t)(0), (uint8_t)(b), (uint8_t)(r), (uint8_t)(g)})
#endif

#ifdef WS2812_PARALLEL
// two bit planes,
// bit planes are effectively a transposed version of the color values of each led of each strip
// the two bit planes are used for double buffering
static led_bit_planes_t led_strips_bitstream[2][LEDS_PER_STRIP] __attribute__((aligned(4)));
static led_color_t led_colors[NMB_STRIPS][LEDS_PER_STRIP] __attribute__((aligned(4)));
#endif
#ifdef WS2812_SINGLE
static led_color_t led_colors[NMB_STRIPS][LEDS_PER_STRIP] __attribute__((aligned(4)));
static led_color_t __led_colors[NMB_STRIPS][LEDS_PER_STRIP] __attribute__((aligned(4)));
#endif

static inline void clear_led_colors()
{
    memset(led_colors, 0, sizeof(led_colors));
#ifdef WS2812_SINGLE
    memset(__led_colors, 0, sizeof(__led_colors));
#endif
}