#pragma once
#include "hardware/pio.h"
#include <string.h>

#include "ws2812.pio.h"
#include "ws2812_defs.h"
#include "ws2812_dma.h"

#ifdef WS2812_SINGLE
static PIO pio[NMB_STRIPS];
static uint sm[NMB_STRIPS];
static uint offset[NMB_STRIPS];
static uint sm_mask = 0;
#endif

bool WS2812_init()
{
#ifdef WS2812_PARALLEL
    PIO pio;
    uint sm;
    uint offset;
    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    auto success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_parallel_program, &pio, &sm, &offset, WS2812_PIN_BASE, NMB_STRIPS, true);
    hard_assert(success);
    ws2812_parallel_program_init(pio, sm, offset, WS2812_PIN_BASE, NMB_STRIPS, 800000);
    sem_init(&ws2812_transmitting_sem, 1, 1); // initially posted so we don't block first time
    ws2812_dma_init(pio, sm);

    return success;
#endif
#ifdef WS2812_SINGLE
    // PIO pio[NMB_STRIPS];
    // uint sm[NMB_STRIPS];
    // uint offset[NMB_STRIPS];

    // uint sm_mask = 0;

    auto success = true;
    for (int i = 0; i < NMB_STRIPS && success; i++)
    {
        success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_single_program, &pio[i], &sm[i], &offset[i], WS2812_PIN_BASE + i, 1, true);
        hard_assert(success);
        ws2812_single_program_init(pio[i], sm[i], offset[i], WS2812_PIN_BASE + i, 800000);
        sm_mask |= 1u << sm[i];
    }
    sem_init(&ws2812_transmitting_sem, 1, 1); // initially posted so we don't block first time
    ws2812_dma_init(pio, sm);

    // check if all sms are on the same PIO and synchronize them
    auto p = pio[0];
    for (int i = 1; i < NMB_STRIPS; i++)
    {
        hard_assert(p == pio[i]);
    }

    // todo consider using this method instead
    // pio_enable_sm_multi_mask_in_sync;
    pio_enable_sm_mask_in_sync(pio[0], sm_mask);

    return success;
#endif
}

#ifdef WS2812_SINGLE
void output_colors()
{
    // disable all state machines
    pio_set_sm_mask_enabled(pio[0], sm_mask, false);

    output_colors_dma();

    bool ready = false;
    while (!ready)
    {
        ready = true;
        for (int i = 0; i < NMB_STRIPS; i++)
        {
            ready &= !pio_sm_is_tx_fifo_empty(pio[i], sm[i]);
        }
    }

    pio_enable_sm_mask_in_sync(pio[0], sm_mask);
}
#endif

#ifdef WS2812_PARALLEL
static inline void led_colors_to_bitplanes_standard(
    led_bit_planes_t *const bitplane,
    const led_color_t *const colors)
{
    memset(bitplane, 0, LEDS_PER_STRIP * sizeof(led_bit_planes_t));

    int color_byte_offset = 0;
    for (int strip = 0; strip < NMB_STRIPS; strip++)
    {
        for (int led = 0; led < LEDS_PER_STRIP; led++, color_byte_offset += sizeof(led_color_t) - BYTES_PER_WS2812_LED)
        {
            for (int c = 0; c < BYTES_PER_WS2812_LED; c++, color_byte_offset++)
            {
                for (int i = 0; i < 8; i++)
                {
                    const uint8_t bit = (((uint8_t *)colors)[color_byte_offset] >> (7 - i)) & 1;
                    if (bit)
                    {
                        const int bitplanes_byte_offset = (led * BYTES_PER_WS2812_LED + c) * 8 + i;
                        ((bit_plane_t *)bitplane)[bitplanes_byte_offset] |= (1 << strip);
                    }
                }
            }
        }
    }
}

static inline void led_colors_to_bitplanes(
    led_bit_planes_t *const bitplane,
    const led_color_t *const colors)
{
    memset(bitplane, 0, LEDS_PER_STRIP * sizeof(led_bit_planes_t));

    const int color_step = sizeof(led_color_t) - BYTES_PER_WS2812_LED;

    uint8_t const *color = (uint8_t const *)colors;
    for (int strip = 0; strip < NMB_STRIPS; strip++)
    {
        const bit_plane_t strip_bit = 1 << strip;
        bit_plane_t *bp = (bit_plane_t *)bitplane + 7;
        for (int led = 0; led < LEDS_PER_STRIP; led++, color += color_step)
        {
            for (int c = 0; c < BYTES_PER_WS2812_LED; c++, color++, bp += 16)
            {
                uint8_t color_byte = *color;
                for (int i = 0; i < 8; i++, color_byte >>= 1, bp--)
                {
                    if (color_byte & 1)
                    {
                        *bp |= strip_bit;
                    }
                }
            }
        }
    }
}
#endif