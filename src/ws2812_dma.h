#pragma once
#include "hardware/dma.h"
#include "pico/sem.h"

#include "ws2812_defs.h"

// ws2812 dma channel; initialized in ws2812_dma_init() function
static int ws2812_dma_channel;

// posted when it is safe to output a new set of values to ws2812
static struct semaphore ws2812_transmitting_sem;

// alarm handle for handling the ws2812 reset delay
static alarm_id_t ws2812_reset_alarm_id;

int64_t ws2812_reset_completed(__unused alarm_id_t id, __unused void *user_data)
{
    ws2812_reset_alarm_id = 0;
    sem_release(&ws2812_transmitting_sem);
    // no repeat
    return 0;
}

void __isr ws2812_dma_complete_handler()
{
    if (dma_hw->ints0 & (1u << ws2812_dma_channel))
    {
        // clear IRQ
        dma_hw->ints0 = (1u << ws2812_dma_channel);
        // when the dma is complete we start the reset delay timer
        if (ws2812_reset_alarm_id)
        {
            cancel_alarm(ws2812_reset_alarm_id);
        }
        ws2812_reset_alarm_id = add_alarm_in_us(WS2812_RESET_US, ws2812_reset_completed, NULL, true);
    }
}

void ws2812_dma_init(PIO pio, uint sm)
{
    ws2812_dma_channel = dma_claim_unused_channel(true);

    dma_channel_config channel_config = dma_channel_get_default_config(ws2812_dma_channel);
    channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_8);
    dma_channel_configure(
        ws2812_dma_channel,
        &channel_config,
        &pio->txf[sm],
        NULL, // set in output_colors_dma
        LEDS_PER_STRIP * BYTES_PER_LED * 8,
        false);

    irq_add_shared_handler(DMA_IRQ_0, ws2812_dma_complete_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    dma_channel_set_irq0_enabled(ws2812_dma_channel, true);
    irq_set_enabled(DMA_IRQ_0, true);
}

void output_colors_dma(int active_planes)
{
    dma_channel_hw_addr(ws2812_dma_channel)->al3_read_addr_trig = (uintptr_t)(led_strips_bitplanes + active_planes);
}