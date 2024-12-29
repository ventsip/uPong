#pragma once
#include "hardware/pio.h"
#include <string.h>

#include "ws2812.pio.h"
#include "ws2812_defs.h"
#include "ws2812_dma.h"

bool init_WS2812()
{
    PIO pio;
    uint sm;
    uint offset;
    bool success;
    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_parallel_program, &pio, &sm, &offset, WS2812_PIN_BASE, NMB_STRIPS, true);
    hard_assert(success);
    ws2812_parallel_program_init(pio, sm, offset, WS2812_PIN_BASE, NMB_STRIPS, 800000);
    sem_init(&ws2812_trasmitting_sem, 1, 1); // initially posted so we don't block first time
    dma_init(pio, sm);

    return success;
}

static inline void set_color(uint8_t *colors, uint8_t r, uint8_t g, uint8_t b)
{
    colors[0] = g;
    colors[1] = r;
    colors[2] = b;
}

static inline void colors_to_bitplanes_standard(
    bit_plane_type *const bitplane,
    const uint8_t *const colors,
    const int nmb_strips,
    const int leds_per_strip,
    const int bytes_per_led)
{
    memset(bitplane, 0, leds_per_strip * bytes_per_led * 8);

    int color_byte_offset = 0;
    for (int y = 0; y < nmb_strips; y++)
    {
        for (int x = 0; x < leds_per_strip; x++)
        {
            for (int c = 0; c < bytes_per_led; c++, color_byte_offset++)
            {
                for (int i = 0; i < 8; i++)
                {
                    const uint8_t bit = (colors[color_byte_offset] >> (7 - i)) & 1;
                    if (bit)
                    {
                        const int bitplanes_byte_offset = (x * bytes_per_led + c) * 8 + i;
                        bitplane[bitplanes_byte_offset] |= (1 << y);
                    }
                }
            }
        }
    }
}

static inline void colors_to_bitplanes(
    bit_plane_type *const bitplane,
    const uint8_t *const colors,
    const int nmb_strips,
    const int leds_per_strip,
    const int bytes_per_led)
{
    memset(bitplane, 0, leds_per_strip * bytes_per_led * 8);

    int color_byte_offset = 0;
    for (int y = 0; y < nmb_strips; y++)
    {
        const bit_plane_type yth_bit = 1 << y;
        int bit_plane_led_offset = 0;
        for (int x = 0; x < leds_per_strip; x++)
        {
            for (int c = 0; c < bytes_per_led; c++, color_byte_offset++, bit_plane_led_offset += 8)
            {
                uint8_t color_byte = colors[color_byte_offset];
                bit_plane_type *bp = bitplane + bit_plane_led_offset + 7;
                for (int i = 0; i < 8; i++, color_byte >>= 1, bp--)
                {
                    if (color_byte & 1)
                    {
                        *bp |= yth_bit;
                    }
                }
            }
        }
    }
}