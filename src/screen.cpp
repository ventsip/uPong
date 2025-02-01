#include <cmath>
#include <hardware/dma.h>
#include <pico/multicore.h>
#include <pico/time.h>

#include "screen.hpp"

namespace screen
{
    // screen buffer
    static ws2812::led_color_t __scr_screen[2][SCREEN_HEIGHT][SCREEN_WIDTH] __attribute__((aligned(4)));
    volatile static uint8_t __scr_screen_active = 0;
    ws2812::led_color_t (*scr_screen)[SCREEN_HEIGHT][SCREEN_WIDTH];
    ws2812::led_color_t (*__scr_screen_buffer)[SCREEN_HEIGHT][SCREEN_WIDTH]; // the screen buffer to send to the led strips
    // posted when it is safe to output a new set of values to ws2812
    static struct semaphore __scr_processing_screen_buffer;
    bool scr_gamma_correction = true;
    bool scr_dither = true;

    // dithering buffers
    static ws2812::led_color_t
        __dth_e[SCREEN_HEIGHT][SCREEN_WIDTH],
        __dth_v[SCREEN_HEIGHT][SCREEN_WIDTH];

    // profile
    volatile scr_profile_t scr_profile;

    void scr_clear_screen()
    {
        memset((void *)scr_screen, 0, sizeof(*scr_screen));
    }

    static uint8_t gamma8_lookup[256];
    static void screen_set_gamma(float gamma)
    {
        for (int i = 0; i < 256; i++)
        {
            gamma8_lookup[i] = (uint8_t)(powf((float)i / 255.0f, gamma) * 255.0f + 0.5f);
        }
    }

    static int _scr_dma_channel = -1;
    const static auto _scr_word_multiple = sizeof(ws2812::led_color_t) % 4 == 0;

    static void __scr_draw_screen();
    static void __scr_screen_draw_loop()
    {
        while (true)
        {
            __scr_draw_screen();
        }
    }

    void scr_screen_init()
    {
        ws2812::WS2812_init();

        screen_set_gamma(2.8);

        memset(__dth_e, 0, sizeof(__dth_e));
        memset(__dth_v, 0, sizeof(__dth_v));

        scr_screen = &(__scr_screen[__scr_screen_active]);
        scr_clear_screen();
        __scr_screen_buffer = &(__scr_screen[1 - __scr_screen_active]);
        memset((void *)__scr_screen_buffer, 0, sizeof(*__scr_screen_buffer));
        sem_init(&__scr_processing_screen_buffer, 1, 1); // initially posted so we don't block first time

        _scr_dma_channel = dma_claim_unused_channel(true);

        const dma_channel_transfer_size transfer_size = _scr_word_multiple ? DMA_SIZE_32 : DMA_SIZE_8;

        dma_channel_config channelConfig = dma_channel_get_default_config(_scr_dma_channel);
        channel_config_set_transfer_data_size(&channelConfig, transfer_size);
        channel_config_set_read_increment(&channelConfig, true);
        channel_config_set_write_increment(&channelConfig, true);
        dma_channel_configure(
            _scr_dma_channel, // Channel to be configured
            &channelConfig,   // The configuration we just created
            NULL,             // The initial write address
            NULL,             // The initial read address
            0,                // Number of transfers; we will set this later
            false);           // Don't start immediately

        multicore_launch_core1(__scr_screen_draw_loop);
    }

    void scr_screen_swap(const bool gamma, const bool dither)
    {
        sem_acquire_blocking(&__scr_processing_screen_buffer);

        scr_gamma_correction = gamma;
        scr_dither = dither;

        __scr_screen_buffer = scr_screen;

        __scr_screen_active ^= 1;
        scr_screen = &(__scr_screen[__scr_screen_active]);

        sem_release(&__scr_processing_screen_buffer);

        scr_clear_screen();
    }

    inline void _gamma_correction()
    {
        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                ws2812::led_color_t *pixel = &((*__scr_screen_buffer)[y][x]);
                pixel->r = gamma8_lookup[pixel->r];
                pixel->g = gamma8_lookup[pixel->g];
                pixel->b = gamma8_lookup[pixel->b];
            }
        }
    }

    inline void _dithering()
    {
        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                __dth_v[y][x].r = ((*__scr_screen_buffer)[y][x].r + __dth_e[y][x].r) >> 1;
                __dth_v[y][x].g = ((*__scr_screen_buffer)[y][x].g + __dth_e[y][x].g) >> 1;
                __dth_v[y][x].b = ((*__scr_screen_buffer)[y][x].b + __dth_e[y][x].b) >> 1;
                __dth_e[y][x].r = ((*__scr_screen_buffer)[y][x].r + __dth_e[y][x].r) - (__dth_v[y][x].r << 1);
                __dth_e[y][x].g = ((*__scr_screen_buffer)[y][x].g + __dth_e[y][x].g) - (__dth_v[y][x].g << 1);
                __dth_e[y][x].b = ((*__scr_screen_buffer)[y][x].b + __dth_e[y][x].b) - (__dth_v[y][x].b << 1);
            }
        }
    }

    static void inline _reverse_copy_pixels_to_led_colors(ws2812::led_color_t *led_colors, const ws2812::led_color_t *pixels, const int n)
    {
        pixels = pixels + n - 1;
        for (int i = 0; i < n; i++)
        {
            *led_colors++ = *pixels--;
        }
    }

    static void inline _forward_copy_pixels_to_led_colors(ws2812::led_color_t *led_colors, const ws2812::led_color_t *pixels, const int n)
    {
        dma_channel_wait_for_finish_blocking(_scr_dma_channel);

        const auto transfer_count = _scr_word_multiple ? n * sizeof(ws2812::led_color_t) / 4 : n * sizeof(ws2812::led_color_t);
        dma_hw->ch[_scr_dma_channel].al2_transfer_count = transfer_count;
        dma_hw->ch[_scr_dma_channel].al2_read_addr = (uint32_t)pixels;
        dma_hw->ch[_scr_dma_channel].al2_write_addr_trig = (uint32_t)led_colors;
    }

    // this function copies the screen buffer to the led_colors buffer, following the specific arrangement of the led matrices
    // ----------------------
    // | S0M1 | S1M1 | S2M1 |
    // |------|------|------|
    // | S0M0 | S1M0 | S2M0 |
    // ----------------------
    // the screen buffer is assumed to be in the same format as the led_colors buffer
    void screen_to_led_colors(ws2812::led_color_t *scr)
    {
        ws2812::led_color_t *pixel_base = scr + (SCREEN_HEIGHT - 1) * SCREEN_WIDTH;
        for (int strip_row = 0; strip_row < ws2812::NMB_STRIP_ROWS; strip_row++)
        {
            ws2812::led_color_t *led = (ws2812::led_color_t *)ws2812::led_colors + strip_row * ws2812::NMB_STRIP_COLUMNS * ws2812::LEDS_PER_STRIP;
            ws2812::led_color_t *pixel = pixel_base - strip_row * ws2812::LED_MATRIX_HEIGHT * SCREEN_WIDTH;
            for (int strip_col = 0; strip_col < ws2812::NMB_STRIP_COLUMNS; strip_col++)
            {
                for (int matrix_row = 0; matrix_row < ws2812::LED_MATRIX_HEIGHT; matrix_row++)
                {
                    if (matrix_row & 1)
                    {
                        _reverse_copy_pixels_to_led_colors(led, pixel, ws2812::LED_MATRIX_WIDTH);
                    }
                    else
                    {
                        _forward_copy_pixels_to_led_colors(led, pixel, ws2812::LED_MATRIX_WIDTH);
                    }
                    led += ws2812::LED_MATRIX_WIDTH;
                    pixel -= SCREEN_WIDTH;
                }
                pixel += ws2812::LED_MATRIX_WIDTH + SCREEN_WIDTH * ws2812::LED_MATRIX_HEIGHT;
            }
        }
    }

#define PROFILE_CALL(func, timer)                                       \
    {                                                                   \
        absolute_time_t start_time = get_absolute_time();               \
        func;                                                           \
        timer = absolute_time_diff_us(start_time, get_absolute_time()); \
    }

    static void __scr_draw_screen()
    {
        sem_acquire_blocking(&__scr_processing_screen_buffer);
#ifdef WS2812_PARALLEL
        static int frame_buffer_index = 0;
#endif

        // apply gamma correction
        PROFILE_CALL(
            scr_gamma_correction ? _gamma_correction() : void(),
            scr_profile.time_gamma_correction);

        // apply dithering
        PROFILE_CALL(
            scr_dither ? _dithering() : void(),
            scr_profile.time_dithering);

        // convert the screen buffer to led colors
        PROFILE_CALL(
            screen_to_led_colors(scr_dither ? (ws2812::led_color_t *)__dth_v : (ws2812::led_color_t *)__scr_screen_buffer),
            scr_profile.time_screen_to_led_colors);

        // convert the colors to bit planes
#ifdef WS2812_PARALLEL
        PROFILE_CALL(
            led_colors_to_bitplanes(ws2812::led_strips_bitstream[frame_buffer_index], (ws2812::led_color_t *)ws2812::led_colors),
            scr_profile.time_led_colors_to_bitplanes);
#endif
        sem_release(&__scr_processing_screen_buffer);

#ifdef WS2812_PARALLEL
        PROFILE_CALL(
            ws2812::transmit_led_colors_dma(frame_buffer_index),
            scr_profile.time_wait_for_DMA);
        frame_buffer_index ^= 1;
#endif
#ifdef WS2812_SINGLE
        PROFILE_CALL(
            ws2812::transmit_led_colors(),
            scr_profile.time_wait_for_DMA);
#endif
    }
}