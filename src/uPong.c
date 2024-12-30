#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>

#include "blink.pio.h"
#include "ws2812_misc.h"

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq)
{
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (clock_get_hz(clk_sys) / (2 * freq)) - 3;
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
    colors_to_bitplanes(bitplane, colors, nmb_strips, leds_per_strip, bytes_per_led);

    // compare the two bit planes
    int ret = memcmp(bitplane_s, bitplane, leds_per_strip * bytes_per_led * 8);
    return ret == 0;
}

void pattern1(int counter, int brightness)
{
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
            //     set_color(&led_colors[(y * LEDS_PER_STRIP + x) * BYTES_PER_LED], 255, 0, 0);
            // }
            // else
            {
                set_color(&led_colors[(y * LEDS_PER_STRIP + x) * BYTES_PER_LED], red, green, blue);
            }
        }
    }
}

void pattern2(int counter, int brightness)
{
    int led = counter % LEDS_PER_STRIP;

    for (int strip = 0; strip < NMB_STRIPS; strip++)
    {
        int red = (strip == 0) ? brightness : 0;
        int green = (strip == 1) ? brightness : 0;
        int blue = (strip == 2) ? brightness : 0;

        for (int i = 1; i <= 8; ++i)
        {
            set_color(&led_colors[(strip * LEDS_PER_STRIP + ((led + LEDS_PER_STRIP - i) % LEDS_PER_STRIP)) * BYTES_PER_LED], red / i, green / i, blue / i);
        }
    }
}

int main()
{
    stdio_init_all();

    int tests = unit_tests();

    PIO pio;
    uint sm;
    uint offset;
    bool success;
    // PIO Blinking LED
    success = pio_claim_free_sm_and_add_program_for_gpio_range(&blink_program, &pio, &sm, &offset, PICO_DEFAULT_LED_PIN, 1, true);
    hard_assert(success);
    blink_pin_forever(pio, sm, offset, PICO_DEFAULT_LED_PIN, 2);

    init_WS2812();

    int frame_buffer_index = 0;
    int counter = 0;
    int brightness = 0;
    int frame = 0;
    int frame_rate = 0;
    absolute_time_t last_time = {0};
    while (true)
    {
        // calculate fps
        frame++;

        absolute_time_t current_time = get_absolute_time();
        if (absolute_time_diff_us(last_time, current_time) >= 1000000)
        {
            frame_rate = frame;
            frame = 0;
            last_time = current_time;
        }

        // clear the led colors
        memset(led_colors, 0, sizeof(led_colors));

        // pattern1(counter, brightness);
        pattern2(counter, brightness);
        counter++;
        brightness = 64; // (brightness + 8) % 64;

        // convert the colors to bit planes
        absolute_time_t start_time = get_absolute_time();
        colors_to_bitplanes(led_strips_bitplanes[frame_buffer_index], led_colors, NMB_STRIPS, LEDS_PER_STRIP, BYTES_PER_LED);
        int64_t time_color_to_bitplanes = absolute_time_diff_us(start_time, get_absolute_time());

        start_time = get_absolute_time();
        sem_acquire_blocking(&ws2812_trasmitting_sem);
        int64_t time_wait_for_DMA = absolute_time_diff_us(start_time, get_absolute_time());

        output_colors_dma(frame_buffer_index);
        // toggle active planes
        frame_buffer_index ^= 1;

        printf("FPS %d; ", frame_rate);
        printf("unit tests %s; ", tests ? "passed" : "failed");
        printf("colors_to_bitplanes: %06lld us; ", time_color_to_bitplanes);
        printf("waited DMA to finish %06lld us\n", time_wait_for_DMA);
    }
}
