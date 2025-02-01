#include "pico/stdlib.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "pong_game.hpp"
#include "rotary_encoder.hpp"
#include "screen.hpp"

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
    // int tests = unit_tests();

    stdio_init_all();
    status_led_init();

    screen::scr_screen_init();
    rotary_encoder::rotary_encoders_init();

    int frame = 0;
    int frame_rate = 0;

    absolute_time_t last_time = {0};

    pong_game::game_init();
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

        pong_game::game_update();
        pong_game::game_draw();

        printf("FPS %d; ", frame_rate);
        // printf("unit tests %s; ", tests ? "passed" : "failed");
        // printf("PIOs/SMs (%ld, %d) (%ld, %d) (%ld, %d); ", (int32_t)pio[0], sm[0], (int32_t)pio[1], sm[1], (int32_t)pio[2], sm[2]);
        printf("gamma_correction: %06lld us; ", screen::scr_profile.time_gamma_correction);
        printf("dithering: %06lld us; ", screen::scr_profile.time_dithering);
        printf("screen_to_led_colors: %06lld us; ", screen::scr_profile.time_screen_to_led_colors);
        printf("led_colors_to_bitplanes: %06lld us; ", screen::scr_profile.time_led_colors_to_bitplanes);
        printf("DMA: %06lld us", screen::scr_profile.time_wait_for_DMA);
        printf("\n");
        frame++;
    }
    pong_game::game_exit();
}
