#pragma once
#include <math.h>

#include "screen_primitives.hpp"
#include "ws2812.hpp"

using namespace screen;

int unit_tests()
{
#ifdef WS2812_PARALLEL
    ws2812::led_color_t colors[ws2812::NMB_STRIPS * ws2812::LEDS_PER_STRIP];
    // initialize colors with random values
    for (int i = 0; i < ws2812::NMB_STRIPS * ws2812::LEDS_PER_STRIP; i++)
    {
        colors[i] = (ws2812::led_color_t){(uint8_t)(rand() % 256), (uint8_t)(rand() % 256), (uint8_t)(rand() % 256), (uint8_t)(rand() % 256)};
    }
    ws2812::led_bit_planes_t bitplane_s[ws2812::LEDS_PER_STRIP];
    ws2812::led_colors_to_bitplanes_standard(bitplane_s, colors);

    ws2812::led_bit_planes_t bitplane[ws2812::LEDS_PER_STRIP];
    ws2812::led_colors_to_bitplanes(bitplane, colors);

    // compare the two bit planes
    int ret = memcmp(bitplane_s, bitplane, ws2812::LEDS_PER_STRIP);
    return ret == 0;
#endif
    // todo
    return true;
}

void led_pattern_1(int counter, uint8_t brightness)
{
    ws2812::clear_led_colors();
    for (int s = 0; s < ws2812::NMB_STRIPS; s++)
    {
        for (auto l = 0; l < ws2812::LEDS_PER_STRIP; l++)
        {
            int color = (counter + l * 64) % (256 + 256 + 256);
            uint8_t red = (color >= 0 && color < 256) ? brightness : 0;
            uint8_t green = (color >= 256 && color < 256 + 256) ? brightness : 0;
            uint8_t blue = (color >= 256 + 256 && color < 256 + 256 + 256) ? brightness : 0;
#ifdef WS2812_PARALLEL
            ws2812::led_colors[s][l] = ws2812_pack_color(red, green, blue);
#endif
#ifdef WS2812_SINGLE
            (*ws2812::led_colors)[s][l] = ws2812_pack_color(red, green, blue);
#endif
        };
    }
}

void led_pattern_2(int counter, int brightness)
{
    ws2812::clear_led_colors();
    int led = counter % ws2812::LEDS_PER_STRIP;

    for (int strip = 0; strip < ws2812::NMB_STRIPS; strip++)
    {
        int red = (strip == 0) ? brightness : 0;
        int green = (strip == 1) ? brightness : 0;
        int blue = (strip == 2) ? brightness : 0;

        for (int i = 1; i <= 8; ++i)
        {
#ifdef WS2812_PARALLEL
            ws2812::led_colors[strip][(led + ws2812::LEDS_PER_STRIP - i) % ws2812::LEDS_PER_STRIP] = ws2812_pack_color(red / i, green / i, blue / i);
#endif
#ifdef WS2812_SINGLE
            (*ws2812::led_colors)[strip][(led + ws2812::LEDS_PER_STRIP - i) % ws2812::LEDS_PER_STRIP] = ws2812_pack_color(red / i, green / i, blue / i);
#endif
        }
    }
}

void screen_pattern_running_pixel(absolute_time_t, int counter, uint8_t brightness)
{
    set_pixel(counter % SCREEN_WIDTH, (counter / SCREEN_WIDTH) % SCREEN_HEIGHT, ws2812_pack_color(0, brightness, 0));
}

void screen_pattern_random_noise(absolute_time_t, int, uint8_t brightness)
{
    // set random pixels
    for (int i = 0; i < 60; i++)
    {
        int x = rand() % SCREEN_WIDTH;
        int y = rand() % SCREEN_HEIGHT;
        set_pixel(x, y, ws2812_pack_color(rand() % brightness, rand() % brightness, rand() % brightness));
    }
}

void screen_pattern_exploding_circle(absolute_time_t, int counter, int brightness)
{
    int x0 = SCREEN_WIDTH / 2;
    int y0 = SCREEN_HEIGHT / 2;
    int radius = (counter / 2) % (SCREEN_WIDTH * 3);

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            int dx = x - x0;
            int dy = y - y0;
            if (dx * dx + dy * dy <= radius * radius)
            {
                auto b = brightness * (dx * dx + dy * dy) / (radius * radius);
                if (b > 20)
                {
                    set_pixel(x, y, ws2812_pack_color(brightness * (dx * dx + dy * dy) / (radius * radius), 0, 0));
                }
            }
        }
    }
}

void screen_pattern_blinking_cursor(absolute_time_t current_time, int, uint8_t brightness)
{
    // blink draw green retangle (3x5) every second
    if (to_us_since_boot(current_time) / 500000 % 2 == 0)
    {
        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 3; x++)
            {
                set_pixel(x + 1, y + 1, ws2812_pack_color(0, brightness, 0));
            }
        }
    }
}

void screen_pattern_uptime_in_ms(absolute_time_t current_time, int, uint8_t brightness)
{
    uint n = to_us_since_boot(current_time) / 1000;

    // get number of digits in n
    int digits = 0;
    for (uint i = n; i > 0; i /= 10)
    {
        digits++;
    }

    // draw transparent rectangle under the number
    draw_transparent_rect(0, SCREEN_HEIGHT - 7, digits * 4 + 1, 7, ws2812_pack_color(0, 0, 0), 200);

    draw_3x5_number(n, 1, SCREEN_HEIGHT - 6, ws2812_pack_color(0, brightness, 0));
}

void screen_pattern_frame_rate(absolute_time_t, int, __unused uint8_t brightness, int frame_rate)
{
    // get number of digits in n
    int digits = 0;
    for (uint i = frame_rate; i > 0; i /= 10)
    {
        digits++;
    }

    digits = std::max(digits, 1);

    // draw transparent rectangle under the number
    draw_transparent_rect(SCREEN_WIDTH - digits * 4 - 1, SCREEN_HEIGHT / 2, digits * 4 + 1, 7, ws2812_pack_color(0, 0, 0), 250);

    draw_3x5_number(frame_rate, SCREEN_WIDTH - digits * 4, SCREEN_HEIGHT / 2 + 1, ws2812_pack_color(32, 32, 32));
}

void screen_pattern_brightness(absolute_time_t, int, uint8_t brightness, __unused int frame_rate)
{
    // get number of digits in n
    int digits = 0;
    for (uint i = brightness; i > 0; i /= 10)
    {
        digits++;
    }

    digits = std::max(digits, 1);

    // draw transparent rectangle under the number
    draw_transparent_rect(SCREEN_WIDTH - digits * 4 - 1, SCREEN_HEIGHT - 7, digits * 4 + 1, 7, ws2812_pack_color(0, 0, 0), 250);

    draw_3x5_number(brightness, SCREEN_WIDTH - digits * 4, SCREEN_HEIGHT - 6, ws2812_pack_color(32, 32, 32));
}

void screen_pattern_bg_flag_transparent(absolute_time_t, int counter, uint8_t brightness)
{
    const int x0 = (counter / 4) % (SCREEN_WIDTH + 24) - 12;
    const int y0 = (counter / 8) % (SCREEN_HEIGHT + 18) - 9;
    // draw transparent bulgarian flag 12x9
    draw_transparent_rect(x0, y0, 12, 3, ws2812_pack_color(brightness, brightness, brightness), 128);
    draw_transparent_rect(x0, y0 + 3, 12, 3, ws2812_pack_color(0, 150 * brightness >> 8, 110 * brightness >> 8), 128);
    draw_transparent_rect(x0, y0 + 6, 12, 3, ws2812_pack_color(214 * brightness >> 8, 38 * brightness >> 8, 18 * brightness >> 8), 128);
}

void screen_pattern_diagonal_line(absolute_time_t, int, uint8_t brightness)
{
    // draw a diagonal line
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        set_pixel(i, i, ws2812_pack_color(0, brightness, 0));
    }
}

void screen_pattern_color_matrices()
{
    for (int i = 0; i < 256; i++)
    {
        set_pixel(i % ws2812::LED_MATRIX_WIDTH, i / ws2812::LED_MATRIX_HEIGHT, ws2812_pack_color(0, i, 0));
        set_pixel(i % ws2812::LED_MATRIX_WIDTH + ws2812::LED_MATRIX_WIDTH, i / ws2812::LED_MATRIX_HEIGHT, ws2812_pack_color(i, 0, 0));
        set_pixel(i % ws2812::LED_MATRIX_WIDTH + ws2812::LED_MATRIX_WIDTH + ws2812::LED_MATRIX_WIDTH, i / ws2812::LED_MATRIX_HEIGHT, ws2812_pack_color(0, 0, i));
        set_pixel(i % ws2812::LED_MATRIX_WIDTH + ws2812::LED_MATRIX_WIDTH, i / ws2812::LED_MATRIX_HEIGHT + ws2812::LED_MATRIX_HEIGHT, ws2812_pack_color(i, i, i));
    }
}

void screen_pattern_three_pixels()
{
    set_pixel(0, SCREEN_HEIGHT - 1, ws2812_pack_color(0, 0, 0));
    set_pixel(ws2812::LED_MATRIX_WIDTH, SCREEN_HEIGHT - 1, ws2812_pack_color(128, 128, 128));
    set_pixel(2 * ws2812::LED_MATRIX_WIDTH, SCREEN_HEIGHT - 1, ws2812_pack_color(255, 255, 255));
}

void screen_pattern_lines_1(absolute_time_t, int counter, int brightness)
{
    // draw line around the screen
    draw_line(0, 0, SCREEN_WIDTH - 1, 0, ws2812_pack_color(brightness, 0, brightness));
    draw_line(0, 0, 0, SCREEN_HEIGHT - 1, ws2812_pack_color(0, brightness, 0));
    draw_line(SCREEN_WIDTH - 1, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, ws2812_pack_color(0, 0, brightness));
    draw_line(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, ws2812_pack_color(0, brightness, brightness));

    // iterate over points of a circle with radius SCREEN_WIDTH
    auto i = counter % 360;
    int x = SCREEN_WIDTH * cos(i * 3.14159 / 180);
    int y = SCREEN_WIDTH * sin(i * 3.14159 / 180);
    draw_line(SCREEN_WIDTH / 2 - x, SCREEN_HEIGHT / 2 - y, SCREEN_WIDTH / 2 + x, SCREEN_HEIGHT / 2 + y, ws2812_pack_color(brightness, brightness, 0));
}

void screen_pattern_scroll_text(absolute_time_t, int counter, int brightness)
{
    counter /= 50;
    // string that contain all characters from code 32 to 126
    static const char text[] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ Well, hello there. Price tag: $25.00. baba.meca@gorata.com";

    // draw text
    int x = SCREEN_WIDTH - 4 * (counter % (12 + strlen(text)));
    draw_3x5_string(text, x, SCREEN_HEIGHT - 24, ws2812_pack_color(brightness, brightness, brightness));
}

void screen_pattern_color_squares(absolute_time_t, __unused int counter, int brightness)
{
    for (int y = 0; y < ws2812::LED_MATRIX_HEIGHT; y++)
    {
        for (int x = 0; x < ws2812::LED_MATRIX_WIDTH; x++)
        {
            set_pixel(x,
                      y,
                      ws2812_pack_color((ws2812::LED_MATRIX_WIDTH - x - 1) * brightness / ws2812::LED_MATRIX_WIDTH,
                                        (ws2812::LED_MATRIX_HEIGHT - y - 1) * brightness / ws2812::LED_MATRIX_HEIGHT,
                                        0));
            set_pixel(x,
                      y + ws2812::LED_MATRIX_HEIGHT,
                      ws2812_pack_color((ws2812::LED_MATRIX_WIDTH - x - 1) * brightness / ws2812::LED_MATRIX_WIDTH,
                                        0,
                                        y * brightness / ws2812::LED_MATRIX_HEIGHT));
            set_pixel(x + ws2812::LED_MATRIX_WIDTH,
                      y,
                      ws2812_pack_color(0,
                                        (ws2812::LED_MATRIX_HEIGHT - y - 1) * brightness / ws2812::LED_MATRIX_HEIGHT,
                                        x * brightness / ws2812::LED_MATRIX_WIDTH));
            set_pixel(x + ws2812::LED_MATRIX_WIDTH,
                      y + ws2812::LED_MATRIX_HEIGHT,
                      ws2812_pack_color(0,
                                        0,
                                        std::min((x + y) * brightness / ((ws2812::LED_MATRIX_WIDTH + ws2812::LED_MATRIX_HEIGHT) / 2), brightness)));

            //           set_pixel(x + LED_MATRIX_WIDTH, y, ws2812_pack_color((LED_MATRIX_WIDTH - x - 1) * brightness / LED_MATRIX_WIDTH, y * brightness / LED_MATRIX_HEIGHT, 0));
        }
    }
}

// Takes:
//   h: hue in degrees (0-360)
//   s: saturation in percent (0-100)
//   v: value in percent (0-100)
// Returns: rgb components in range 0-255 via pointers
void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b)
{
    // Convert inputs to 0-1 range
    float hue = h / 360.0f;
    float sat = s / 100.0f;
    float val = v / 100.0f;

    // Calculate intermediate values
    float c = val * sat; // Chroma
    float x = c * (1 - fabsf(fmodf(hue * 6, 2) - 1));
    float m = val - c;

    float r_tmp, g_tmp, b_tmp;

    if (hue < 1.0f / 6.0f)
    { // 0-60° - red to yellow
        r_tmp = c;
        g_tmp = x;
        b_tmp = 0;
    }
    else if (hue < 2.0f / 6.0f)
    { // 60-120° - yellow to green
        r_tmp = x;
        g_tmp = c;
        b_tmp = 0;
    }
    else if (hue < 3.0f / 6.0f)
    { // 120-180° - green to cyan
        r_tmp = 0;
        g_tmp = c;
        b_tmp = x;
    }
    else if (hue < 4.0f / 6.0f)
    { // 180-240° - cyan to blue
        r_tmp = 0;
        g_tmp = x;
        b_tmp = c;
    }
    else if (hue < 5.0f / 6.0f)
    { // 240-300° - blue to magenta
        r_tmp = x;
        g_tmp = 0;
        b_tmp = c;
    }
    else
    { // 300-360° - magenta to red
        r_tmp = c;
        g_tmp = 0;
        b_tmp = x;
    }

    // Add value offset and convert to 0-255 range
    *r = (uint8_t)((r_tmp + m) * 255);
    *g = (uint8_t)((g_tmp + m) * 255);
    *b = (uint8_t)((b_tmp + m) * 255);
}

void screen_pattern_color_HSV_square(absolute_time_t, __unused int counter, int brightness)
{
    counter /= 4;

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            uint8_t r, g, b;
            hsv_to_rgb(x * 360 / SCREEN_WIDTH, 100, y * 100 / SCREEN_HEIGHT, &r, &g, &b);
            set_pixel((SCREEN_WIDTH + x + counter % SCREEN_WIDTH) % SCREEN_WIDTH, y, ws2812_pack_color(r * brightness / 255, g * brightness / 255, b * brightness / 255));
        }
    }
}