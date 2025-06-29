#pragma once

#include "screen_mock.hpp"
#include "ws2812_mock.hpp"

// Mock version of screen_primitives.hpp for host testing

namespace screen
{
    static inline void set_pixel(const int x, const int y, const ws2812::led_color_t c)
    {
        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        {
            (*scr_screen)[y][x] = c;
        }
    }

    static inline void set_pixel(const int x, const int y, const ws2812::led_color_t c, const uint8_t alpha)
    {
        if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
        {
            const uint16_t r_scaled = c.r * alpha;
            const uint16_t g_scaled = c.g * alpha;
            const uint16_t b_scaled = c.b * alpha;

            const uint8_t anti_alpha = 255 - alpha;
            ws2812::led_color_t *p = &(*scr_screen)[y][x];

            p->g = (p->g * anti_alpha + g_scaled) >> 8;
            p->r = (p->r * anti_alpha + r_scaled) >> 8;
            p->b = (p->b * anti_alpha + b_scaled) >> 8;
        }
    }

    static inline void draw_vertical_line(const int x, int y0, int y1, const ws2812::led_color_t c)
    {
        if (x < 0 || x >= SCREEN_WIDTH) return;
        if (y0 > y1) { int tmp = y0; y0 = y1; y1 = tmp; }
        if (y1 < 0 || y0 >= SCREEN_HEIGHT) return;
        if (y0 < 0) y0 = 0;
        if (y1 >= SCREEN_HEIGHT) y1 = SCREEN_HEIGHT - 1;
        for (int y = y0; y <= y1; y++)
        {
            (*scr_screen)[y][x] = c;
        }
    }

    static inline void draw_horizontal_line(const int y, int x0, int x1, const ws2812::led_color_t c)
    {
        if (y < 0 || y >= SCREEN_HEIGHT) return;
        if (x0 > x1) { int tmp = x0; x0 = x1; x1 = tmp; }
        if (x1 < 0 || x0 >= SCREEN_WIDTH) return;
        if (x0 < 0) x0 = 0;
        if (x1 >= SCREEN_WIDTH) x1 = SCREEN_WIDTH - 1;
        for (int x = x0; x <= x1; x++)
        {
            (*scr_screen)[y][x] = c;
        }
    }

    static inline void draw_rect(int x, int y, int w, int h, const ws2812::led_color_t c)
    {
        if (x < 0) { w += x; x = 0; }
        if (y < 0) { h += y; y = 0; }
        if (x + w > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
        if (y + h > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;
        if (w <= 0 || h <= 0) return;

        for (int i = y; i < y + h; i++)
        {
            for (int j = x; j < x + w; j++)
            {
                (*scr_screen)[i][j] = c;
            }
        }
    }

    static inline void draw_orb(const float x_c, const float y_c, const float radius, const ws2812::led_color_t c)
    {
        if (x_c + radius < 0 || x_c - radius >= SCREEN_WIDTH || y_c + radius < 0 || y_c - radius >= SCREEN_HEIGHT)
        {
            return;
        }
        if (radius <= 0)
        {
            set_pixel((int)x_c, (int)y_c, c);
            return;
        }

        for (int x = x_c - radius; x <= x_c + radius + 1; x++)
        {
            for (int y = y_c - radius; y <= y_c + radius + 1; y++)
            {
                const float d = ((x - x_c) * (x - x_c) + (y - y_c) * (y - y_c)) / (radius * radius);
                if (d <= 1)
                {
                    set_pixel(x, y, c, (1 - d) * 255);
                }
            }
        }
    }

    enum font_3x5_alignment_t
    {
        FONT_3X5_CENTER = 0,
        FONT_3X5_LEFT = 1,
        FONT_3X5_RIGHT = 2
    };

    // Mock implementations for font rendering (simplified for testing)
    void draw_3x5_char(const char ch, const int x, int y, const ws2812::led_color_t c);
    void draw_3x5_string(const char *str, const int x, const int y, const ws2812::led_color_t c, const font_3x5_alignment_t alignment = FONT_3X5_LEFT);
    void draw_3x5_number(const unsigned int number, const int x, const int y, const ws2812::led_color_t c, const font_3x5_alignment_t alignment = FONT_3X5_LEFT);
}