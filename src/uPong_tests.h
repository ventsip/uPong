#include "screen_defs.h"
#include "ws2812_defs.h"
#include "ws2812_misc.h"

int unit_tests()
{
#ifdef WS2812_PARALLEL
    led_color_t colors[NMB_STRIPS * LEDS_PER_STRIP];
    // initialize colors with random values
    for (int i = 0; i < NMB_STRIPS * LEDS_PER_STRIP; i++)
    {
        colors[i] = (led_color_t){(uint8_t)(rand() % 256), (uint8_t)(rand() % 256), (uint8_t)(rand() % 256), (uint8_t)(rand() % 256)};
    }
    led_bit_planes_t bitplane_s[LEDS_PER_STRIP];
    led_colors_to_bitplanes_standard(bitplane_s, colors);

    led_bit_planes_t bitplane[LEDS_PER_STRIP];
    led_colors_to_bitplanes(bitplane, colors);

    // compare the two bit planes
    int ret = memcmp(bitplane_s, bitplane, LEDS_PER_STRIP);
    return ret == 0;
#endif
    // todo
    return true;
}

void led_pattern_1(int counter, uint8_t brightness)
{
    clear_led_colors();
    for (int s = 0; s < NMB_STRIPS; s++)
    {
        for (int l = 0; l < LEDS_PER_STRIP; l++)
        {
            int color = (counter + l * 64) % (256 + 256 + 256);
            uint8_t red = (color >= 0 && color < 256) ? brightness : 0;
            uint8_t green = (color >= 256 && color < 256 + 256) ? brightness : 0;
            uint8_t blue = (color >= 256 + 256 && color < 256 + 256 + 256) ? brightness : 0;

            led_colors[s][l] = ws2812_pack_color(red, green, blue);
        };
    }
}

void led_pattern_2(int counter, int brightness)
{
    clear_led_colors();
    int led = counter % LEDS_PER_STRIP;

    for (int strip = 0; strip < NMB_STRIPS; strip++)
    {
        int red = (strip == 0) ? brightness : 0;
        int green = (strip == 1) ? brightness : 0;
        int blue = (strip == 2) ? brightness : 0;

        for (int i = 1; i <= 8; ++i)
        {
            led_colors[strip][(led + LEDS_PER_STRIP - i) % LEDS_PER_STRIP] = ws2812_pack_color(red / i, green / i, blue / i);
        }
    }
}

void screen_pattern_1(absolute_time_t, int counter, uint8_t brightness)
{
    set_pixel(counter % SCREEN_WIDTH, (counter / SCREEN_WIDTH) % SCREEN_HEIGHT, ws2812_pack_color(0, brightness, 0));
}

void screen_pattern_2(absolute_time_t, int, uint8_t brightness)
{
    // set random pixels
    for (int i = 0; i < 60; i++)
    {
        int x = rand() % SCREEN_WIDTH;
        int y = rand() % SCREEN_HEIGHT;
        set_pixel(x, y, ws2812_pack_color(rand() % brightness, rand() % brightness, rand() % brightness));
    }
}

void screen_pattern_3(absolute_time_t, int counter, int brightness)
{
    int x0 = SCREEN_WIDTH / 2;
    int y0 = SCREEN_HEIGHT / 2;
    int radius = (counter / 2) % (SCREEN_WIDTH * 2);

    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            int dx = x - x0;
            int dy = y - y0;
            if (dx * dx + dy * dy <= radius * radius)
            {
                set_pixel(x, y, ws2812_pack_color(brightness * (dx * dx + dy * dy) / (radius * radius), 0, 0));
            }
        }
    }
}

void screen_pattern_4(absolute_time_t current_time, int, uint8_t brightness)
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

void screen_pattern_5(absolute_time_t current_time, int, uint8_t brightness)
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

void screen_pattern_6(absolute_time_t, int, uint8_t brightness, int frame_rate)
{
    // get number of digits in n
    int digits = 0;
    for (uint i = frame_rate; i > 0; i /= 10)
    {
        digits++;
    }

    // draw transparent rectangle under the number
    draw_transparent_rect(SCREEN_WIDTH - digits * 4 - 1, 0, digits * 4 + 1, 7, ws2812_pack_color(brightness, brightness, brightness), 128);

    draw_3x5_number(frame_rate, SCREEN_WIDTH - digits * 4, 1, ws2812_pack_color(0, 0, 0));
}

void screen_pattern_7(absolute_time_t, int counter, uint8_t brightness)
{
    const int x0 = (counter / 4) % (SCREEN_WIDTH + 24) - 12;
    const int y0 = (counter / 8) % (SCREEN_HEIGHT + 18) - 9;
    // draw transparent bulgarian flag 12x9
    draw_transparent_rect(x0, y0, 12, 3, ws2812_pack_color(brightness, brightness, brightness), 128);
    draw_transparent_rect(x0, y0 + 3, 12, 3, ws2812_pack_color(0, 150 * brightness >> 8, 110 * brightness >> 8), 128);
    draw_transparent_rect(x0, y0 + 6, 12, 3, ws2812_pack_color(214 * brightness >> 8, 38 * brightness >> 8, 18 * brightness >> 8), 128);
}

void screen_pattern_8(absolute_time_t, int, uint8_t brightness)
{
    // draw a diagonal line
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        set_pixel(i, i, ws2812_pack_color(0, brightness, 0));
    }
}

void screen_pattern_8()
{
    // draw a diagonal line
    for (int i = 0; i < 256; i++)
    {
        set_pixel(i % LED_MATRIX_WIDTH, i / LED_MATRIX_HEIGHT, ws2812_pack_color(0, i, 0));
        set_pixel(i % LED_MATRIX_WIDTH + LED_MATRIX_WIDTH, i / LED_MATRIX_HEIGHT, ws2812_pack_color(i, 0, 0));
        set_pixel(i % LED_MATRIX_WIDTH + LED_MATRIX_WIDTH + LED_MATRIX_WIDTH, i / LED_MATRIX_HEIGHT, ws2812_pack_color(0, 0, i));
        set_pixel(i % LED_MATRIX_WIDTH + LED_MATRIX_WIDTH, i / LED_MATRIX_HEIGHT + LED_MATRIX_HEIGHT, ws2812_pack_color(i, i, i));
    }
}
