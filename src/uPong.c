#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pico/sem.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blink.pio.h"
#include "ws2812.pio.h"

#define WS2812_RESET_US 100
#define WS2812_PIN_BASE 2
#if WS2812_PIN_BASE >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

#define LEDS_PER_STRIP (16 * 16 * 2)
#define NMB_STRIPS 3 // max 8 strips
#if NMB_STRIPS > 8
#error "NMB_STRIPS must be <= 8"
#endif
#if NMB_STRIPS <= 8
typedef uint8_t bit_plane_type; // must be wide enough to contain the number of strips
#endif

#define BYTES_PER_LED 3

inline void colors_to_bitplanes_standard(
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
// two bit planes, each consists of LEDS_PER_STRIP * BYTES_PER_LED elements of bit_plane_type
// bit planes are effectivley a transposed version of the color values of each led of each strip
// the two bit planes are used for double buffering
bit_plane_type led_strips_bitplanes[2][LEDS_PER_STRIP * BYTES_PER_LED * 8];
uint8_t led_colors[NMB_STRIPS * LEDS_PER_STRIP * BYTES_PER_LED]; // color order is GRB (WS2812)

inline void colors_to_bitplanes(
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

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq)
{
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (clock_get_hz(clk_sys) / (2 * freq)) - 3;
}

// bit plane content dma channel
#define DMA_CHANNEL 0
#define DMA_CHANNEL_MASK (1u << DMA_CHANNEL)
#define DMA_CHANNELS_MASK (DMA_CHANNEL_MASK)

// posted when it is safe to output a new set of values
static struct semaphore ws2812_trasmitting_sem;

// alarm handle for handling delay
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

void set_color(uint8_t *colors, uint8_t r, uint8_t g, uint8_t b)
{
    colors[0] = g;
    colors[1] = r;
    colors[2] = b;
}

int unit_tests()
{
    const int nmb_strips = NMB_STRIPS, leds_per_strip = LEDS_PER_STRIP, bytes_per_led = BYTES_PER_LED;

    uint8_t colors[nmb_strips * leds_per_strip * bytes_per_led];
    // initialize colors with random values
    for (int i = 0; i < nmb_strips * leds_per_strip * bytes_per_led; i++)
    {
        colors[i] = rand() % 256;
    }
    bit_plane_type bitplane_s[leds_per_strip * bytes_per_led * 8];
    colors_to_bitplanes_standard(bitplane_s, colors, nmb_strips, leds_per_strip, bytes_per_led);

    bit_plane_type bitplane[leds_per_strip * bytes_per_led * 8];
    colors_to_bitplanes(bitplane, colors, nmb_strips, leds_per_strip, bytes_per_led);

    // compare the two bit planes
    int ret = memcmp(bitplane_s, bitplane, leds_per_strip * bytes_per_led * 8);
    return ret == 0;
}

int main()
{
    stdio_init_all();

    int tests = unit_tests();

    PIO pio;
    uint sm;
    uint offset;
    bool success;
    // PIO Blinking LED
    success = pio_claim_free_sm_and_add_program_for_gpio_range(&blink_program, &pio, &sm, &offset, PICO_DEFAULT_LED_PIN, 1, true);
    hard_assert(success);
    blink_pin_forever(pio, sm, offset, PICO_DEFAULT_LED_PIN, 2);

    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_parallel_program, &pio, &sm, &offset, WS2812_PIN_BASE, NMB_STRIPS, true);
    hard_assert(success);

    ws2812_parallel_program_init(pio, sm, offset, WS2812_PIN_BASE, NMB_STRIPS, 800000);
    sem_init(&ws2812_trasmitting_sem, 1, 1); // initially posted so we don't block first time
    dma_init(pio, sm);

    int frame_buffer_index = 0;
    int color = 0;
    int brightness = 0;
    int frame = 0;
    int frame_rate = 0;
    absolute_time_t last_time = {0};
    while (true)
    {
        // calculate fps
        frame++;

        absolute_time_t current_time = get_absolute_time();
        if (absolute_time_diff_us(last_time, current_time) >= 1000000)
        {
            frame_rate = frame;
            frame = 0;
            last_time = current_time;
        }

        int red = (color >= 0 && color < 256) ? brightness : 0;
        int green = (color >= 256 && color < 256 + 256) ? brightness : 0;
        int blue = (color >= 256 + 256 && color < 256 + 256 + 256) ? brightness : 0;
        color = (color + 1) % (256 + 256 + 256);
        brightness = (brightness + 1) % 256;

        for (int y = 0; y < NMB_STRIPS; y++)
        {
            for (int x = 0; x < LEDS_PER_STRIP; x++)
            {
                if (x == y)
                {
                    set_color(&led_colors[(y * LEDS_PER_STRIP + x) * BYTES_PER_LED], 255, 0, 0);
                }
                else
                {
                    set_color(&led_colors[(y * LEDS_PER_STRIP + x) * BYTES_PER_LED], red, green, blue);
                }
            }
        }

        // convert the colors to bit planes
        absolute_time_t start_time = get_absolute_time();
        colors_to_bitplanes(led_strips_bitplanes[frame_buffer_index], led_colors, NMB_STRIPS, LEDS_PER_STRIP, BYTES_PER_LED);
        int64_t time_color_to_bitplanes = absolute_time_diff_us(start_time, get_absolute_time());

        start_time = get_absolute_time();
        sem_acquire_blocking(&ws2812_trasmitting_sem);
        int64_t time_wait_for_DMA = absolute_time_diff_us(start_time, get_absolute_time());

        output_colors_dma(frame_buffer_index);
        // toggle active planes
        frame_buffer_index ^= 1;

        printf("FPS %d; ", frame_rate);
        printf("unit tests %s; ", tests ? "passed" : "failed");
        printf("R:%03d G:%03d  B:%03d; ", red, green, blue);
        printf("colors_to_bitplanes: %lld us; ", time_color_to_bitplanes);
        printf("waited DMA to finish %lld us\n", time_wait_for_DMA);
    }
}
