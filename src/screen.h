#pragma once
#include <string.h>

#include "ws2812_defs.h"

#define SCREEN_WIDTH (LED_MATRIX_WIDTH * 3)
#define SCREEN_HEIGHT (LED_MATRIX_HEIGHT * 2)
#define BYTES_PER_PIXEL 3 // 3 bytes per pixel, color order is GRB, following the WS2812 standard
#if BYTES_PER_PIXEL != BYTES_PER_LED
#error "BYTES_PER_PIXEL must be equal to BYTES_PER_LED"
#endif

// screen buffer
static uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL];

static inline void clear_screen()
{
    memset(screen, 0, sizeof(screen));
}

static inline void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
    {
        uint8_t *p = screen + (y * SCREEN_WIDTH + x) * BYTES_PER_PIXEL;
        *p++ = g;
        *p++ = r;
        *p = b;
    }
}

static void inline reverse_copy_pixels_to_led_colors(uint8_t *led_colors, const uint8_t *pixels, int n)
{
    uint8_t *led = led_colors;
    const uint8_t *pixel = pixels + (n - 1) * BYTES_PER_PIXEL;
    for (int i = 0; i < n; i++, pixel -= 2 * BYTES_PER_PIXEL)
    {
        *led++ = *pixel++;
        *led++ = *pixel++;
        *led++ = *pixel++;
    }
}

// this function copies the screen buffer to the led_colors buffer, following the specific arrangement of the led matrices
// ----------------------
// | S0M1 | S1M1 | S2M1 |
// |------|------|------|
// | S0M0 | S1M0 | S2M0 |
// ----------------------
// the screen buffer is assumed to be in the same format as the led_colors buffer
static void screen_to_led_colors()
{
    for (int y = 0, inv_y = SCREEN_HEIGHT - 1; y < SCREEN_HEIGHT; y++, inv_y--)
    {
        uint8_t *pixel = screen + y * SCREEN_WIDTH * BYTES_PER_PIXEL;
        uint8_t *led = led_colors + inv_y * LED_MATRIX_WIDTH * BYTES_PER_PIXEL;

        for (int i = 0; i < 3; ++i, pixel += LED_MATRIX_WIDTH * BYTES_PER_PIXEL, led += LEDS_PER_STRIP * BYTES_PER_LED)
        {
            if (inv_y & 1)
            {
                reverse_copy_pixels_to_led_colors(led, pixel, LED_MATRIX_WIDTH);
            }
            else
            {
                memcpy(led, pixel, LED_MATRIX_WIDTH * BYTES_PER_PIXEL);
            }
        }
    }
}