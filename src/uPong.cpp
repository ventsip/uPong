#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>

#define WS2812_SINGLE

#include "screen_misc.h"
#include "uPong_tests.h"
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

int main()
{
    int tests = unit_tests();

    stdio_init_all();
    status_led_init();

    WS2812_init();
    screen_init();
#ifdef WS2812_PARALLEL
    int frame_buffer_index = 0;
#endif
    int counter = 0;
    int brightness = 32;
    int frame = 0;
    int frame_rate = 0;
    absolute_time_t last_time = {0};
    absolute_time_t start_time = {0};
    while (true)
    {
        set_status_led(frame & 1);

        absolute_time_t current_time = get_absolute_time();
        if (absolute_time_diff_us(last_time, current_time) >= 1000000)
        {
            frame_rate = frame;
            frame = 0;
            last_time = current_time;
        }

        clear_screen();
        screen_pattern_1(current_time, counter, brightness);
        screen_pattern_3(current_time, counter, brightness);
        screen_pattern_2(current_time, counter, brightness);
        screen_pattern_4(current_time, counter, brightness);
        screen_pattern_5(current_time, counter, brightness);
        screen_pattern_7(current_time, counter, brightness);
        screen_pattern_6(current_time, counter, brightness, frame_rate);
        // screen_pattern_8();

        // convert the screen buffer to led colors
        start_time = get_absolute_time();
        screen_to_led_colors();
        int64_t time_screen_to_led_colors = absolute_time_diff_us(start_time, get_absolute_time());

        // led_pattern_2(counter, brightness);

        // convert the colors to bit planes
        start_time = get_absolute_time();
#ifdef WS2812_PARALLEL
        led_colors_to_bitplanes(led_strips_bitstream[frame_buffer_index], (led_color_t *)led_colors);
#endif
        int64_t time_led_colors_to_bitplanes = absolute_time_diff_us(start_time, get_absolute_time());

        start_time = get_absolute_time();
        sem_acquire_blocking(&ws2812_transmitting_sem);
        int64_t time_wait_for_DMA = absolute_time_diff_us(start_time, get_absolute_time());
#ifdef WS2812_PARALLEL
        output_colors_dma(frame_buffer_index);
        frame_buffer_index ^= 1;
#endif
#ifdef WS2812_SINGLE
        output_colors_dma();
#endif
        // toggle active planes

        printf("FPS %d; ", frame_rate);
        printf("unit tests %s; ", tests ? "passed" : "failed");
        printf("screen_to_led_colors: %06lld us; ", time_screen_to_led_colors);
        printf("led_colors_to_bitplanes: %06lld us; ", time_led_colors_to_bitplanes);
        printf("waited DMA to finish %06lld us\n", time_wait_for_DMA);

        counter++;
        frame++;
    }
}
