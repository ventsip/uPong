#include <cmath>
#include <hardware/dma.h>

#include "screen.hpp"
#include "ws2812.hpp"

namespace screen
{
    // screen buffer
    ws2812::led_color_t scr_screen[SCREEN_HEIGHT][SCREEN_WIDTH] __attribute__((aligned(4)));
    void clear_screen()
    {
        memset(scr_screen, 0, sizeof(scr_screen));
    }

    uint8_t gamma8_lookup[256];
    static void screen_set_gamma(float gamma)
    {
        for (int i = 0; i < 256; i++)
        {
            gamma8_lookup[i] = (uint8_t)(powf((float)i / 255.0f, gamma) * 255.0f + 0.5f);
        }
    }

    static int scr_dma_channel = -1;
    const static auto word_multiple = sizeof(ws2812::led_color_t) % 4 == 0;

    void screen_init()
    {
        ws2812::WS2812_init();

        screen_set_gamma(2.8);

        clear_screen();

        scr_dma_channel = dma_claim_unused_channel(true);

        const dma_channel_transfer_size transfer_size = word_multiple ? DMA_SIZE_32 : DMA_SIZE_8;

        dma_channel_config channelConfig = dma_channel_get_default_config(scr_dma_channel);
        channel_config_set_transfer_data_size(&channelConfig, transfer_size);
        channel_config_set_read_increment(&channelConfig, true);
        channel_config_set_write_increment(&channelConfig, true);
        dma_channel_configure(
            scr_dma_channel, // Channel to be configured
            &channelConfig,  // The configuration we just created
            NULL,            // The initial write address
            NULL,            // The initial read address
            0,               // Number of transfers; we will set this later
            false);          // Don't start immediately
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
        dma_channel_wait_for_finish_blocking(scr_dma_channel);

        const auto transfer_count = word_multiple ? n * sizeof(ws2812::led_color_t) / 4 : n * sizeof(ws2812::led_color_t);
        dma_hw->ch[scr_dma_channel].al2_transfer_count = transfer_count;
        dma_hw->ch[scr_dma_channel].al2_read_addr = (uint32_t)pixels;
        dma_hw->ch[scr_dma_channel].al2_write_addr_trig = (uint32_t)led_colors;
    }

    // this function copies the screen buffer to the led_colors buffer, following the specific arrangement of the led matrices
    // ----------------------
    // | S0M1 | S1M1 | S2M1 |
    // |------|------|------|
    // | S0M0 | S1M0 | S2M0 |
    // ----------------------
    // the screen buffer is assumed to be in the same format as the led_colors buffer
    void screen_to_led_colors(const bool gamma_correction)
    {
        // apply gamma correction
        if (gamma_correction)
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

        // apply some dithering
        static ws2812::led_color_t
            e[SCREEN_HEIGHT][SCREEN_WIDTH],
            v[SCREEN_HEIGHT][SCREEN_WIDTH];
        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                v[y][x].r = (scr_screen[y][x].r + e[y][x].r) >> 1;
                v[y][x].g = (scr_screen[y][x].g + e[y][x].g) >> 1;
                v[y][x].b = (scr_screen[y][x].b + e[y][x].b) >> 1;
                e[y][x].r = (scr_screen[y][x].r + e[y][x].r) - (v[y][x].r << 1);
                e[y][x].g = (scr_screen[y][x].g + e[y][x].g) - (v[y][x].g << 1);
                e[y][x].b = (scr_screen[y][x].b + e[y][x].b) - (v[y][x].b << 1);
            }
        }

        const auto scr = v;

        for (int strip_row = 0; strip_row < ws2812::NMB_STRIP_ROWS; strip_row++)
        {
            ws2812::led_color_t *led = (ws2812::led_color_t *)ws2812::led_colors + strip_row * ws2812::NMB_STRIP_COLUMNS * ws2812::LEDS_PER_STRIP;
            ws2812::led_color_t *pixel = (ws2812::led_color_t *)(scr[SCREEN_HEIGHT - 1 - strip_row * ws2812::LED_MATRIX_HEIGHT]);
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
}