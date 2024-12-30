#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>

#include "screen.h"
#include "ws2812_misc.h"

// Initialize the GPIO for the LED
void status_led_init(void)
{
#ifdef PICO_DEFAULT_LED_PIN
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif
}

// Turn the LED on or off
void set_status_led(bool led_on)
{
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#endif
}

int unit_tests()
{
    const int nmb_strips = NMB_STRIPS, leds_per_strip = LEDS_PER_STRIP, bytes_per_led = BYTES_PER_LED;

    uint8_t colors[nmb_strips * leds_per_strip * bytes_per_led];
    // initialize colors with random values
    for (int i = 0; i < nmb_strips * leds_per_strip * bytes_per_led; i++)
    {
        colors[i] = rand() % 256;
    }
    bit_plane_type bitplane_s[leds_per_strip * bytes_per_led * 8];
    colors_to_bitplanes_standard(bitplane_s, colors, nmb_strips, leds_per_strip, bytes_per_led);

    bit_plane_type bitplane[leds_per_strip * bytes_per_led * 8];
    led_colors_to_bitplanes(bitplane, colors, nmb_strips, leds_per_strip, bytes_per_led);

    // compare the two bit planes
    int ret = memcmp(bitplane_s, bitplane, leds_per_strip * bytes_per_led * 8);
    return ret == 0;
}

void led_pattern_1(int counter, int brightness)
{
    clear_led_colors();
    for (int y = 0; y < NMB_STRIPS; y++)
    {
        for (int x = 0; x < LEDS_PER_STRIP; x++)
        {
            int color = (counter + x * 64) % (256 + 256 + 256);
            int red = (color >= 0 && color < 256) ? brightness : 0;
            int green = (color >= 256 && color < 256 + 256) ? brightness : 0;
            int blue = (color >= 256 + 256 && color < 256 + 256 + 256) ? brightness : 0;

            // if (x == y)
            // {
            //     set_led_color(&led_colors[(y * LEDS_PER_STRIP + x) * BYTES_PER_LED], 255, 0, 0);
            // }
            // else
            {
                set_led_color(&led_colors[(y * LEDS_PER_STRIP + x) * BYTES_PER_LED], red, green, blue);
            }
        }
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
            set_led_color(&led_colors[(strip * LEDS_PER_STRIP + ((led + LEDS_PER_STRIP - i) % LEDS_PER_STRIP)) * BYTES_PER_LED], red / i, green / i, blue / i);
        }
    }
}

void screen_pattern_1(absolute_time_t current_time, int counter, int brightness)
{
    set_pixel(counter % SCREEN_WIDTH, (counter / SCREEN_WIDTH) % SCREEN_HEIGHT, 0, brightness, 0);
}

void screen_pattern_2(absolute_time_t current_time, int counter, int brightness)
{
    // set random pixels
    for (int i = 0; i < 60; i++)
    {
        int x = rand() % SCREEN_WIDTH;
        int y = rand() % SCREEN_HEIGHT;
        set_pixel(x, y, rand() % brightness, rand() % brightness, rand() % brightness);
    }
}

void screen_pattern_3(absolute_time_t current_time, int counter, int brightness)
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
                set_pixel(x, y, brightness * (dx * dx + dy * dy) / (radius * radius), 0, 0);
            }
        }
    }
}

void screen_pattern_4(absolute_time_t current_time, int counter, int brightness)
{
    // blink draw green retangle (3x5) every second
    if (to_us_since_boot(current_time) / 500000 % 2 == 0)
    {
        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 3; x++)
            {
                set_pixel(x + 1, y + 1, 0, brightness, 0);
            }
        }
    }
}

void screen_pattern_5(absolute_time_t current_time, int counter, int brightness)
{
    uint n = to_us_since_boot(current_time) / 1000;

    // get number of digits in n
    int digits = 0;
    for (uint i = n; i > 0; i /= 10)
    {
        digits++;
    }

    // draw transparent rectangle under the number
    draw_transparent_rect(0, SCREEN_HEIGHT - 7, digits * 4 + 1, 7, 0, 0, 0, 200);

    draw_3x5_number(n, 1, SCREEN_HEIGHT - 6, 0, brightness, 0);
}

int main()
{
    stdio_init_all();
    status_led_init();

    init_WS2812();

    int frame_buffer_index = 0;
    int counter = 0;
    int brightness = 32;
    int frame = 0;
    int frame_rate = 0;
    absolute_time_t last_time = {0};
    absolute_time_t start_time = {0};
    while (true)
    {
        // calculate fps
        frame++;
        set_status_led(frame & 1);

        absolute_time_t current_time = get_absolute_time();
        if (absolute_time_diff_us(last_time, current_time) >= 1000000)
        {
            frame_rate = frame;
            frame = 0;
            last_time = current_time;
        }

        clear_screen();
        screen_pattern_3(current_time, counter, brightness);
        screen_pattern_2(current_time, counter, brightness);
        screen_pattern_4(current_time, counter, brightness);
        screen_pattern_5(current_time, counter, brightness);

        // convert the screen buffer to led colors
        start_time = get_absolute_time();
        screen_to_led_colors();
        int64_t time_screen_to_led_colors = absolute_time_diff_us(start_time, get_absolute_time());

        // convert the colors to bit planes
        start_time = get_absolute_time();
        led_colors_to_bitplanes(led_strips_bitplanes[frame_buffer_index], led_colors, NMB_STRIPS, LEDS_PER_STRIP, BYTES_PER_LED);
        int64_t time_led_colors_to_bitplanes = absolute_time_diff_us(start_time, get_absolute_time());

        start_time = get_absolute_time();
        sem_acquire_blocking(&ws2812_trasmitting_sem);
        int64_t time_wait_for_DMA = absolute_time_diff_us(start_time, get_absolute_time());

        output_colors_dma(frame_buffer_index);
        // toggle active planes
        frame_buffer_index ^= 1;

        printf("FPS %d; ", frame_rate);
        printf("screen_to_led_colors: %06lld us; ", time_screen_to_led_colors);
        printf("led_colors_to_bitplanes: %06lld us; ", time_led_colors_to_bitplanes);
        printf("waited DMA to finish %06lld us\n", time_wait_for_DMA);

        counter++;
    }
}
