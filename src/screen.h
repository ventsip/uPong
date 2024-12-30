#pragma once
#include <string.h>

#include "fonts.h"
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

static inline void set_pixel(const int x, const int y, const uint8_t r, const uint8_t g, const uint8_t b)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
    {
        uint8_t *p = screen + (y * SCREEN_WIDTH + x) * BYTES_PER_PIXEL;
        *p++ = g;
        *p++ = r;
        *p = b;
    }
}

static inline void draw_transparent_rect(int x, int y, int w, int h, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t alpha)
{
    // fix cooridnates to be within the screen
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (x + w > SCREEN_WIDTH)
    {
        w = SCREEN_WIDTH - x;
    }
    if (y + h > SCREEN_HEIGHT)
    {
        h = SCREEN_HEIGHT - y;
    }
    if (w <= 0 || h <= 0)
    {
        return;
    }

    const uint16_t r_scaled = r * alpha;
    const uint16_t g_scaled = g * alpha;
    const uint16_t b_scaled = b * alpha;

    for (int i = 0; i < h; i++)
    {
        const uint8_t anti_alpha = 255 - alpha;
        uint8_t *p = screen + ((y + i) * SCREEN_WIDTH + x) * BYTES_PER_PIXEL;

        for (int j = 0; j < w; j++)
        {
            *p = (*p * anti_alpha + g_scaled) >> 8;
            p++;
            *p = (*p * anti_alpha + r_scaled) >> 8;
            p++;
            *p = (*p * anti_alpha + b_scaled) >> 8;
            p++;
        }
    }
}

static void inline reverse_copy_pixels_to_led_colors(uint8_t *led_colors, const uint8_t *pixels, const int n)
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

//

// draw a 3x5 digit at the specified position
// x and y are considered to be the top left corner of the digit
void draw_3x5_digit(const char d, const int x, int y, const uint8_t r, const uint8_t g, const uint8_t b)
{
    const uint8_t *digit = (d < '0' || d > '9') ? font_3x5_missing_char : font_3x5_digits[d - '0'];

    for (int row = 0; row < 5; row++, y++)
    {
        const uint8_t line = digit[row];

        if (line & 0b100)
        {
            set_pixel(x, y, r, g, b);
        }
        if (line & 0b010)
        {
            set_pixel(x + 1, y, r, g, b);
        }
        if (line & 0b001)
        {
            set_pixel(x + 2, y, r, g, b);
        }
    }
}

void draw_3x5_number_as_string(const char *str, const int x, const int y, const uint8_t r, const uint8_t g, const uint8_t b)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        draw_3x5_digit(str[i], x + i * 4, y, r, g, b);
    }
}

void draw_3x5_number(const uint number, const int x, const int y, const uint8_t r, const uint8_t g, const uint8_t b)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%u", number);
    draw_3x5_number_as_string(buffer, x, y, r, g, b);
}