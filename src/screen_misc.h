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

static inline void set_pixel(const int x, const int y, const led_color_t c)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
    {
        scr_screen[y][x] = c;
    }
}

static inline void draw_transparent_rect(int x, int y, int w, int h, const led_color_t c, const uint8_t alpha)
{
    // fix coordinates to be within the screen
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if (x + w > SCREEN_WIDTH)
    {
        w = SCREEN_WIDTH - x;
    }
    if (y + h > SCREEN_HEIGHT)
    {
        h = SCREEN_HEIGHT - y;
    }
    if (w <= 0 || h <= 0)
    {
        return;
    }

    const uint16_t r_scaled = c.r * alpha;
    const uint16_t g_scaled = c.g * alpha;
    const uint16_t b_scaled = c.b * alpha;

    for (int i = 0; i < h; i++)
    {
        const uint8_t anti_alpha = 255 - alpha;
        led_color_t *p = &scr_screen[y + i][x];

        for (int j = 0; j < w; j++)
        {
            p->g = (p->g * anti_alpha + g_scaled) >> 8;
            p->r = (p->r * anti_alpha + r_scaled) >> 8;
            p->b = (p->b * anti_alpha + b_scaled) >> 8;
            p++;
        }
    }
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

// draw a 3x5 digit at the specified position
// x and y are considered to be the top left corner of the digit
void draw_3x5_digit(const char d, const int x, int y, const led_color_t c)
{
    const uint8_t *digit = (d < '0' || d > '9') ? font_3x5_missing_char : font_3x5_digits[d - '0'];

    for (int row = 0; row < 5; row++, y++)
    {
        const uint8_t line = digit[row];

        if (line & 0b100)
        {
            set_pixel(x, y, c);
        }
        if (line & 0b010)
        {
            set_pixel(x + 1, y, c);
        }
        if (line & 0b001)
        {
            set_pixel(x + 2, y, c);
        }
    }
}

void draw_3x5_number_as_string(const char *str, const int x, const int y, const led_color_t c)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        draw_3x5_digit(str[i], x + i * 4, y, c);
    }
}

void draw_3x5_number(const uint number, const int x, const int y, const led_color_t c)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%u", number);
    draw_3x5_number_as_string(buffer, x, y, c);
}