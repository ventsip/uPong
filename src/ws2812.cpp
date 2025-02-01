#include <hardware/dma.h>
#include <hardware/pio.h>
#include <pico/mutex.h>
#include <string.h>

#include "ws2812.hpp"
#include "ws2812.pio.h"

namespace ws2812
{
    static const auto WS2812_RESET_US = 80;
    static const auto WS2812_PIN_BASE = 2;

#if WS2812_PIN_BASE >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#ifdef WS2812_PARALLEL
    // two bit planes,
    // bit planes are effectively a transposed version of the color values of each led of each strip
    // the two bit planes are used for double buffering
    led_bit_planes_t led_strips_bitstream[2][LEDS_PER_STRIP] __attribute__((aligned(4)));
    led_color_t led_colors[NMB_STRIPS][LEDS_PER_STRIP] __attribute__((aligned(4)));
    static auto led_colors_size = sizeof(led_colors);
#endif
#ifdef WS2812_SINGLE
    static led_color_t __led_colors[2][NMB_STRIPS][LEDS_PER_STRIP] __attribute__((aligned(4)));
    volatile static int __led_colors_active = 0;
    led_color_t (*led_colors)[NMB_STRIPS][LEDS_PER_STRIP] = &(__led_colors[__led_colors_active]);
    static const auto led_colors_size = sizeof(__led_colors) / 2;
#endif

    void clear_led_colors()
    {
        memset(led_colors, 0, led_colors_size);
    }

#ifdef WS2812_PARALLEL
    // ws2812 dma channel; initialized in ws2812_dma_init() function
    static int ws2812_dma_channel;
#endif
#ifdef WS2812_SINGLE
    // ws2812 dma channel; initialized in ws2812_dma_init() function
    static int ws2812_dma_channels[NMB_STRIPS];
#endif
    static unsigned int ws2812_dma_mask = 0;

    // posted when it is safe to output a new set of values to ws2812
    mutex_t __mutex_transmitting_led_colors;

    // alarm handle for handling the ws2812 reset delay
    static alarm_id_t ws2812_reset_alarm_id = 0;

    int64_t ws2812_reset_completed(__unused alarm_id_t id, __unused void *user_data)
    {
        ws2812_reset_alarm_id = 0;
        mutex_exit(&__mutex_transmitting_led_colors);
        // no repeat
        return 0;
    }

    void __isr ws2812_dma_complete_handler()
    {
        if (dma_hw->ints0 & ws2812_dma_mask)
        {
            // clear IRQ
            dma_hw->ints0 = ws2812_dma_mask;

            // when the dma is complete we start the reset delay timer
            if (ws2812_reset_alarm_id)
            {
                cancel_alarm(ws2812_reset_alarm_id);
            }
            // the DMA have completed
            // wait for the SM to complete sending all bits and also wait for the reset delay
#ifdef WS2812_PARALLEL
            // assume that the SM fifo is full (8 bytes) and one byte is in progress. Add time to wait for all bytes to be sent
            ws2812_reset_alarm_id = add_alarm_in_us(WS2812_RESET_US + (8 + 1) * 1.25, ws2812_reset_completed, NULL, true);
#endif
#ifdef WS2812_SINGLE
            // assume that the SM fifo is full (8 words) and one word is in progress. Add time to wait for all bits (3 * 8) to be sent
            ws2812_reset_alarm_id = add_alarm_in_us(WS2812_RESET_US + (8 + 1) * 3 * 8 * 1.25, ws2812_reset_completed, NULL, true);
#endif
        }
    }

#ifdef WS2812_PARALLEL
    void ws2812_dma_init(PIO pio, uint sm)
    {
        ws2812_dma_channel = dma_claim_unused_channel(true);
        ws2812_dma_mask = 1u << ws2812_dma_channel;

        dma_channel_config channel_config = dma_channel_get_default_config(ws2812_dma_channel);
        channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
        channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_8);
        dma_channel_configure(
            ws2812_dma_channel,
            &channel_config,
            &pio->txf[sm],
            NULL, // set in transmit_led_colors_dma
            LEDS_PER_STRIP * sizeof(led_bit_planes_t),
            false);

        irq_add_shared_handler(DMA_IRQ_0, ws2812_dma_complete_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        dma_channel_set_irq0_enabled(ws2812_dma_channel, true);
        irq_set_enabled(DMA_IRQ_0, true);
    }
#endif

#ifdef WS2812_SINGLE
    void ws2812_dma_init(PIO pio[NMB_STRIPS], uint sm[NMB_STRIPS])
    {
        for (int i = 0; i < NMB_STRIPS; i++)
        {
            ws2812_dma_channels[i] = dma_claim_unused_channel(true);

            dma_channel_config channel_config = dma_channel_get_default_config(ws2812_dma_channels[i]);
            channel_config_set_dreq(&channel_config, pio_get_dreq(pio[i], sm[i], true));
            channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_32);
            channel_config_set_irq_quiet(&channel_config, i != 0);

            dma_channel_configure(
                ws2812_dma_channels[i],
                &channel_config,
                &pio[i]->txf[sm[i]],
                NULL,
                LEDS_PER_STRIP,
                false);
        }

        // irq works with the first channel only
        ws2812_dma_mask |= 1u << ws2812_dma_channels[0];

        dma_channel_set_irq0_enabled(ws2812_dma_channels[0], true);
        irq_add_shared_handler(DMA_IRQ_0, ws2812_dma_complete_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
        irq_set_enabled(DMA_IRQ_0, true);
    }
#endif

#ifdef WS2812_PARALLEL
    void transmit_led_colors_dma(int active_planes)
    {
        sem_acquire_blocking(&__mutex_transmitting_led_colors);
        dma_channel_hw_addr(ws2812_dma_channel)
            ->al3_read_addr_trig = (uintptr_t)(led_strips_bitstream + active_planes);
    }
#endif

#ifdef WS2812_SINGLE
    static PIO pio[NMB_STRIPS];
    static uint sm[NMB_STRIPS];
    static uint offset[NMB_STRIPS];
    static uint sm_mask[NUM_PIOS];
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
        sem_init(&__mutex_transmitting_led_colors, 1, 1); // initially posted so we don't block first time
        ws2812_dma_init(pio, sm);

        return success;
#endif
#ifdef WS2812_SINGLE
        for (auto i = 0u; i < NUM_PIOS; i++)
        {
            sm_mask[i] = 0;
        }

        auto success = true;
        for (auto i = 0; i < NMB_STRIPS && success; i++)
        {
            success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_single_program, &pio[i], &sm[i], &offset[i], WS2812_PIN_BASE + i, 1, true);
            hard_assert(success);
            ws2812_single_program_init(pio[i], sm[i], offset[i], WS2812_PIN_BASE + i, 800000);

            sm_mask[pio_get_index(pio[i])] |= 1u << sm[i];
        }
        mutex_init(&__mutex_transmitting_led_colors);
        ws2812_dma_init(pio, sm);

        // enable all state machines in sync
        hard_assert(sizeof(sm_mask) / sizeof(uint) == 3);
        pio_enable_sm_multi_mask_in_sync(pio1, sm_mask[0], sm_mask[1], sm_mask[2]);

        return success;
#endif
    }

#ifdef WS2812_SINGLE
    void transmit_led_colors()
    {
        mutex_enter_blocking(&__mutex_transmitting_led_colors);

        hard_assert(sizeof(sm_mask) / sizeof(uint) == 3);
        // disable all state machines
        pio_set_sm_multi_mask_enabled(pio1, sm_mask[0], sm_mask[1], sm_mask[2], false);

        // configure and start the DMA channels
        uint32_t dma_all_channel_mask = 0;
        for (int i = 0; i < NMB_STRIPS; i++)
        {
            dma_channel_set_read_addr(ws2812_dma_channels[i], __led_colors[__led_colors_active][i], false);
            dma_all_channel_mask |= 1u << ws2812_dma_channels[i];
        }
        dma_start_channel_mask(dma_all_channel_mask);

        // swap the led_colors buffer
        __led_colors_active ^= 1;
        led_colors = &(__led_colors[__led_colors_active]);

        // wait until all state machines have non-empty TX FIFOs
        bool ready = false;
        while (!ready)
        {
            ready = true;
            for (int i = 0; i < NMB_STRIPS; i++)
            {
                ready &= !pio_sm_is_tx_fifo_empty(pio[i], sm[i]);
            }
        }

        // enable all state machines in sync
        pio_enable_sm_multi_mask_in_sync(pio1, sm_mask[0], sm_mask[1], sm_mask[2]);
    }
#endif

#ifdef WS2812_PARALLEL
    void led_colors_to_bitplanes_standard(
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

    void led_colors_to_bitplanes(
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
}