#pragma once

#include "hardware/dma.h"

#include "fonts.h"
#include "screen_defs.h"

// screen buffer
static led_color_t scr_screen[SCREEN_HEIGHT][SCREEN_WIDTH] __attribute__((aligned(4)));
static inline void clear_screen()
{
    memset(scr_screen, 0, sizeof(scr_screen));
}

static int scr_dma_channel = -1;

void screen_init()
{
    clear_screen();

    scr_dma_channel = dma_claim_unused_channel(true);

    const auto word_multiple = (sizeof(led_color_t) * LED_MATRIX_WIDTH) % 4 == 0;
    const dma_channel_transfer_size transfer_size = word_multiple ? DMA_SIZE_32 : DMA_SIZE_8;
    const auto transfer_count = word_multiple ? LED_MATRIX_WIDTH * sizeof(led_color_t) / 4 : LED_MATRIX_WIDTH * sizeof(led_color_t);

    dma_channel_config channelConfig = dma_channel_get_default_config(scr_dma_channel);
    channel_config_set_transfer_data_size(&channelConfig, transfer_size);
    channel_config_set_read_increment(&channelConfig, true);
    channel_config_set_write_increment(&channelConfig, true);
    dma_channel_configure(
        scr_dma_channel, // Channel to be configured
        &channelConfig,  // The configuration we just created
        NULL,            // The initial write address
        NULL,            // The initial read address
        transfer_count,
        false); // Don't start immediately
}

static void inline reverse_copy_pixels_to_led_colors(led_color_t *led_colors, const led_color_t *pixels, const int n)
{
    pixels = pixels + n - 1;
    for (int i = 0; i < n; i++)
    {
        *led_colors++ = *pixels--;
    }
}

// this function copies the screen buffer to the led_colors buffer, following the specific arrangement of the led matrices
// ----------------------
// | S0M1 | S1M1 | S2M1 |
// |------|------|------|
// | S0M0 | S1M0 | S2M0 |
// ----------------------
// the screen buffer is assumed to be in the same format as the led_colors buffer
static void screen_to_led_colors()
{
    for (int y = 0, inv_y = SCREEN_HEIGHT - 1; y < SCREEN_HEIGHT; y++, inv_y--)
    {
        led_color_t *pixel = (led_color_t *)(scr_screen[y]);
        led_color_t *led = (led_color_t *)led_colors + inv_y * LED_MATRIX_WIDTH;

        for (int strip = 0; strip < NMB_STRIPS; ++strip, pixel += LED_MATRIX_WIDTH, led += LEDS_PER_STRIP)
        {
            if (inv_y & 1)
            {
                reverse_copy_pixels_to_led_colors(led, pixel, LED_MATRIX_WIDTH);
            }
            else
            {
                dma_channel_wait_for_finish_blocking(scr_dma_channel);
                dma_hw->ch[scr_dma_channel].al2_read_addr = (uint32_t)pixel;
                dma_hw->ch[scr_dma_channel].al2_write_addr_trig = (uint32_t)led;
            }
        }
    }
}