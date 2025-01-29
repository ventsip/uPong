#include "pico/stdlib.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "rotary_encoder.h"
#include "screen_primitives.h"
#include "uPong_tests.h"

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

    rotary_encoder::configure_rotary_encoders();

    int counter = 0;
    int32_t rotary_1_pos = 0;
    int32_t rotary_2_pos = 0;
    uint8_t sw_1_state = rotary_encoder::ROTARY_ENCODER_SW_RELEASED;
    uint8_t sw_2_state = rotary_encoder::ROTARY_ENCODER_SW_RELEASED;
    // uint8_t sw_2_state = ROTARY_ENCODER_SW_RELEASED;
    const unsigned int base_brightness = 255;
    unsigned int brightness = base_brightness;
    int frame = 0;
    int frame_rate = 0;
    absolute_time_t last_time = {0};

    while (true)
    {
        set_status_led(frame & 1);

        auto c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT)
        {
            if (c == '+')
            {
                brightness = (brightness + 1) % 256;
            }
            if (c == '-')
            {
                brightness = (brightness - 1) % 256;
            }
        }

        absolute_time_t current_time = get_absolute_time();
        if (absolute_time_diff_us(last_time, current_time) >= 1000000)
        {
            frame_rate = frame;
            frame = 0;
            last_time = current_time;
        }

        screen::scr_clear_screen();

        // screen_pattern_running_pixel(current_time, counter, brightness);
        // screen_pattern_random_noise(current_time, counter, brightness);
        // screen_pattern_blinking_cursor(current_time, counter, brightness);

        // screen_pattern_bg_flag_transparent(current_time, counter, brightness);

        // screen_pattern_lines_1(current_time, counter, brightness);
        // screen_pattern_color_squares(current_time, counter, brightness);
        // screen_pattern_scroll_text(current_time, counter, brightness);
        screen_pattern_color_HSV_square(current_time, rotary_1_pos, brightness);
        screen_pattern_exploding_circle(current_time, counter, brightness);
        screen_pattern_frame_rate(current_time, counter, brightness, frame_rate);
        screen_pattern_brightness(current_time, counter, brightness, frame_rate);
        screen_pattern_uptime_in_ms(current_time, counter, brightness);
        // screen_pattern_three_pixels();
        // screen_pattern_color_matrices();

        scr_draw_screen(sw_1_state == rotary_encoder::ROTARY_ENCODER_SW_RELEASED, sw_2_state == rotary_encoder::ROTARY_ENCODER_SW_RELEASED);

        int32_t rotary_1_delta = rotary_encoder::rotary_encoder_fetch_counter(&rotary_encoder::rotary_encoders[0]);
        rotary_1_pos += rotary_1_delta;
        sw_1_state = rotary_encoder::rotary_encoder_fetch_sw_state(&rotary_encoder::rotary_encoders[0]);

        int32_t rotary_2_delta = rotary_encoder::rotary_encoder_fetch_counter(&rotary_encoder::rotary_encoders[1]);
        rotary_2_pos += rotary_2_delta;
        sw_2_state = rotary_encoder::rotary_encoder_fetch_sw_state(&rotary_encoder::rotary_encoders[1]);

        brightness = (base_brightness + rotary_2_pos / 4) % 256;
        // sw_2_state = rotary_encoder_fetch_sw_state(&rotary_encoders[1]);

        // printf("rotary_1_pos %06ld ", rotary_1_pos);
        // printf("(counter_1_diff %03ld); ", rotary_1_delta);
        // printf("sw_1_state %d; ", sw_1_state);
        // printf("rotary_2_pos %06ld ", rotary_2_pos);
        // printf("(counter_2_diff %03ld); ", rotary_2_delta);
        // printf("sw_2_state %d; ", sw_2_state);
        // printf("(brightness %03d); ", brightness);

        printf("FPS %d; ", frame_rate);
        // printf("unit tests %s; ", tests ? "passed" : "failed");
        // printf("PIOs/SMs (%ld, %d) (%ld, %d) (%ld, %d); ", (int32_t)pio[0], sm[0], (int32_t)pio[1], sm[1], (int32_t)pio[2], sm[2]);
        printf("screen_to_led_colors: %06lld us; ", scr_profile.time_screen_to_led_colors);
        printf("led_colors_to_bitplanes: %06lld us; ", scr_profile.time_led_colors_to_bitplanes);
        printf("waited DMA to finish %06lld us", scr_profile.time_wait_for_DMA);
        printf("\n");
        counter++;
        frame++;
    }
}
