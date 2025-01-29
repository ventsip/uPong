#include <cmath>
#include <hardware/dma.h>

#include "screen.hpp"
#include <pico/time.h>

namespace screen
{
    // screen buffer
    ws2812::led_color_t scr_screen[SCREEN_HEIGHT][SCREEN_WIDTH] __attribute__((aligned(4)));

    // dithering buffers
    static ws2812::led_color_t
        _dither_e[SCREEN_HEIGHT][SCREEN_WIDTH],
        _dither_v[SCREEN_HEIGHT][SCREEN_WIDTH];

    // profile
    volatile scr_profile_t scr_profile;

    void scr_clear_screen()
    {
        memset(scr_screen, 0, sizeof(scr_screen));
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

    void scr_screen_init()
    {
        ws2812::WS2812_init();

        screen_set_gamma(2.8);

        memset(_dither_e, 0, sizeof(_dither_e));
        memset(_dither_v, 0, sizeof(_dither_v));

        scr_clear_screen();

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
    }

    static void inline reverse_copy_pixels_to_led_colors(ws2812::led_color_t *led_colors, const ws2812::led_color_t *pixels, const int n)
    {
        pixels = pixels + n - 1;
        for (int i = 0; i < n; i++)
        {
            *led_colors++ = *pixels--;
        }
    }

    static void inline forward_copy_pixels_to_led_colors(ws2812::led_color_t *led_colors, const ws2812::led_color_t *pixels, const int n)
    {
        dma_channel_wait_for_finish_blocking(_scr_dma_channel);

        const auto transfer_count = _scr_word_multiple ? n * sizeof(ws2812::led_color_t) / 4 : n * sizeof(ws2812::led_color_t);
        dma_hw->ch[_scr_dma_channel].al2_transfer_count = transfer_count;
        dma_hw->ch[_scr_dma_channel].al2_read_addr = (uint32_t)pixels;
        dma_hw->ch[_scr_dma_channel].al2_write_addr_trig = (uint32_t)led_colors;
    }

    inline void gamma_correction()
    {
        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                ws2812::led_color_t *pixel = &scr_screen[y][x];
                pixel->r = gamma8_lookup[pixel->r];
                pixel->g = gamma8_lookup[pixel->g];
                pixel->b = gamma8_lookup[pixel->b];
            }
        }
    }

    inline void dithering()
    {
        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                _dither_v[y][x].r = (scr_screen[y][x].r + _dither_e[y][x].r) >> 1;
                _dither_v[y][x].g = (scr_screen[y][x].g + _dither_e[y][x].g) >> 1;
                _dither_v[y][x].b = (scr_screen[y][x].b + _dither_e[y][x].b) >> 1;
                _dither_e[y][x].r = (scr_screen[y][x].r + _dither_e[y][x].r) - (_dither_v[y][x].r << 1);
                _dither_e[y][x].g = (scr_screen[y][x].g + _dither_e[y][x].g) - (_dither_v[y][x].g << 1);
                _dither_e[y][x].b = (scr_screen[y][x].b + _dither_e[y][x].b) - (_dither_v[y][x].b << 1);
            }
        }
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
                        reverse_copy_pixels_to_led_colors(led, pixel, ws2812::LED_MATRIX_WIDTH);
                    }
                    else
                    {
                        forward_copy_pixels_to_led_colors(led, pixel, ws2812::LED_MATRIX_WIDTH);
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

    void scr_draw_screen(const bool gamma, const bool dither)
    {

#ifdef WS2812_PARALLEL
        static int frame_buffer_index = 0;
#endif

        // apply gamma correction
        PROFILE_CALL(
            gamma ? gamma_correction() : void(),
            scr_profile.time_gamma_correction);

        // apply dithering
        PROFILE_CALL(
            dither ? dithering() : void(),
            scr_profile.time_dithering);

        // convert the screen buffer to led colors
        PROFILE_CALL(
            screen_to_led_colors(dither ? (ws2812::led_color_t *)_dither_v : (ws2812::led_color_t *)scr_screen),
            scr_profile.time_screen_to_led_colors);

        // convert the colors to bit planes
#ifdef WS2812_PARALLEL
        PROFILE_CALL(
            led_colors_to_bitplanes(ws2812::led_strips_bitstream[frame_buffer_index], (ws2812::led_color_t *)ws2812::led_colors),
            scr_profile.time_led_colors_to_bitplanes);
#endif

        PROFILE_CALL(
            ws2812::wait_for_led_colors_transmission(),
            scr_profile.time_wait_for_DMA);

#ifdef WS2812_PARALLEL
        ws2812::transmit_led_colors_dma(frame_buffer_index);
        frame_buffer_index ^= 1;
#endif
#ifdef WS2812_SINGLE
        ws2812::transmit_led_colors();
#endif
    }
}