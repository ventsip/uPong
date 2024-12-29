#pragma once
#include "hardware/dma.h"
#include "pico/sem.h"

#include "ws2812_defs.h"

#define DMA_CHANNEL 0
#define DMA_CHANNEL_MASK (1u << DMA_CHANNEL)
#define DMA_CHANNELS_MASK (DMA_CHANNEL_MASK)

// posted when it is safe to output a new set of values to ws2812
static struct semaphore ws2812_trasmitting_sem;

// alarm handle for handling the ws2812 reset delay
alarm_id_t ws2812_reset_alarm_id;

int64_t ws2812_reset_completed(__unused alarm_id_t id, __unused void *user_data)
{
    ws2812_reset_alarm_id = 0;
    sem_release(&ws2812_trasmitting_sem);
    // no repeat
    return 0;
}

void __isr dma_complete_handler()
{
    if (dma_hw->ints0 & DMA_CHANNEL_MASK)
    {
        // clear IRQ
        dma_hw->ints0 = DMA_CHANNEL_MASK;
        // when the dma is complete we start the reset delay timer
        if (ws2812_reset_alarm_id)
        {
            cancel_alarm(ws2812_reset_alarm_id);
        }
        ws2812_reset_alarm_id = add_alarm_in_us(WS2812_RESET_US, ws2812_reset_completed, NULL, true);
    }
}

void dma_init(PIO pio, uint sm)
{
    dma_claim_mask(DMA_CHANNELS_MASK);

    dma_channel_config channel_config = dma_channel_get_default_config(DMA_CHANNEL);
    channel_config_set_dreq(&channel_config, pio_get_dreq(pio, sm, true));
    channel_config_set_transfer_data_size(&channel_config, DMA_SIZE_8);
    dma_channel_configure(
        DMA_CHANNEL,
        &channel_config,
        &pio->txf[sm],
        NULL, // set in output_colors_dma
        LEDS_PER_STRIP * BYTES_PER_LED * 8,
        false);

    irq_set_exclusive_handler(DMA_IRQ_0, dma_complete_handler);
    dma_channel_set_irq0_enabled(DMA_CHANNEL, true);
    irq_set_enabled(DMA_IRQ_0, true);
}

void output_colors_dma(int active_planes)
{
    dma_channel_hw_addr(DMA_CHANNEL)->al3_read_addr_trig = (uintptr_t)(led_strips_bitplanes + active_planes);
}