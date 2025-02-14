#pragma once

#include <cstdio>
#include <pico/types.h>
#include <stdlib.h>

#include "fonts.hpp"
#include "screen.hpp"
#include "ws2812.hpp"

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
            volatile ws2812::led_color_t *p = &(*scr_screen)[y][x];

            p->g = (p->g * anti_alpha + g_scaled) >> 8;
            p->r = (p->r * anti_alpha + r_scaled) >> 8;
            p->b = (p->b * anti_alpha + b_scaled) >> 8;
        }
    }

    static inline void draw_vertical_line(const int x, int y0, int y1, const ws2812::led_color_t c)
    {
        if (x < 0 || x >= SCREEN_WIDTH)
        {
            return;
        }
        if (y0 > y1)
        {
            int tmp = y0;
            y0 = y1;
            y1 = tmp;
        }
        if (y1 < 0 || y0 >= SCREEN_HEIGHT)
        {
            return;
        }
        if (y0 < 0)
        {
            y0 = 0;
        }
        if (y1 >= SCREEN_HEIGHT)
        {
            y1 = SCREEN_HEIGHT - 1;
        }
        for (int y = y0; y <= y1; y++)
        {
            (*scr_screen)[y][x] = c;
        }
    }

    static inline void draw_horizontal_line(const int y, int x0, int x1, const ws2812::led_color_t c)
    {
        if (y < 0 || y >= SCREEN_HEIGHT)
        {
            return;
        }
        if (x0 > x1)
        {
            int tmp = x0;
            x0 = x1;
            x1 = tmp;
        }
        if (x1 < 0 || x0 >= SCREEN_WIDTH)
        {
            return;
        }
        if (x0 < 0)
        {
            x0 = 0;
        }
        if (x1 >= SCREEN_WIDTH)
        {
            x1 = SCREEN_WIDTH - 1;
        }
        for (int x = x0; x <= x1; x++)
        {
            (*scr_screen)[y][x] = c;
        }
    }

    static inline void draw_line(int x0, int y0, int x1, int y1, const ws2812::led_color_t c)
    {
        if (x0 == x1)
        {
            draw_vertical_line(x0, y0, y1, c);
            return;
        }

        if (y0 == y1)
        {
            draw_horizontal_line(y0, x0, x1, c);
            return;
        }

        // exit if the line is not visible
        if ((x0 < 0 && x1 < 0) ||
            (y0 < 0 && y1 < 0) ||
            (x0 >= SCREEN_WIDTH && x1 >= SCREEN_WIDTH) ||
            (y0 >= SCREEN_HEIGHT && y1 >= SCREEN_HEIGHT))
        {
            return;
        }

        // swap x coordinates if needed
        if (x0 > x1)
        {
            int tmp = x0;
            x0 = x1;
            x1 = tmp;
            tmp = y0;
            y0 = y1;
            y1 = tmp;
        }

        // clip the line to the screen
        if (x0 < 0)
        {
            y0 += (0.0 - x0) * (y1 - y0) / (x1 - x0);
            x0 = 0;
        }
        if (x1 >= SCREEN_WIDTH)
        {
            y1 -= (x1 - SCREEN_WIDTH + 1.0) * (y1 - y0) / (x1 - x0);
            x1 = SCREEN_WIDTH - 1;
        }

        // swap y coordinates if needed
        if (y0 > y1)
        {
            int tmp = y0;
            y0 = y1;
            y1 = tmp;
            tmp = x0;
            x0 = x1;
            x1 = tmp;
        }

        // clip the line to the screen
        if (y0 < 0)
        {
            x0 += (0.0 - y0) * (x1 - x0) / (y1 - y0);
            y0 = 0;
        }
        if (y1 >= SCREEN_HEIGHT)
        {
            x1 -= (y1 - SCREEN_HEIGHT + 1.0) * (x1 - x0) / (y1 - y0);
            y1 = SCREEN_HEIGHT - 1;
        }

        // naive Bresenham's line algorithm
        const int dx = abs(x1 - x0);
        const int dy = abs(y1 - y0);
        const int sx = x0 < x1 ? 1 : -1;
        const int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;

        while (true)
        {
            set_pixel(x0, y0, c);
            if (x0 == x1 && y0 == y1)
            {
                break;
            }
            const int e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx)
            {
                err += dx;
                y0 += sy;
            }
        }

        return;
    }

#define FIX_RECT_COORDS(x, y, w, h, screen_width, screen_height) \
    if (x < 0)                                                   \
    {                                                            \
        w += x;                                                  \
        x = 0;                                                   \
    }                                                            \
    if (y < 0)                                                   \
    {                                                            \
        h += y;                                                  \
        y = 0;                                                   \
    }                                                            \
    if (x + w > screen_width)                                    \
    {                                                            \
        w = screen_width - x;                                    \
    }                                                            \
    if (y + h > screen_height)                                   \
    {                                                            \
        h = screen_height - y;                                   \
    }                                                            \
    if (w <= 0 || h <= 0)                                        \
    {                                                            \
        return;                                                  \
    }

    static inline void draw_rect(int x, int y, int w, int h, const ws2812::led_color_t c)
    {
        // fix coordinates to be within the screen
        FIX_RECT_COORDS(x, y, w, h, SCREEN_WIDTH, SCREEN_HEIGHT)

        for (int i = y; i < y + h; i++)
        {
            for (int j = x; j < x + w; j++)
            {
                (*scr_screen)[i][j] = c;
            }
        }
    }

    static inline void draw_transparent_rect(int x, int y, int w, int h, const ws2812::led_color_t c, const uint8_t alpha)
    {
        // fix coordinates to be within the screen
        FIX_RECT_COORDS(x, y, w, h, SCREEN_WIDTH, SCREEN_HEIGHT)

        const uint16_t r_scaled = c.r * alpha;
        const uint16_t g_scaled = c.g * alpha;
        const uint16_t b_scaled = c.b * alpha;

        for (int i = 0; i < h; i++)
        {
            const uint8_t anti_alpha = 255 - alpha;
            volatile ws2812::led_color_t *p = &(*scr_screen)[y + i][x];

            for (int j = 0; j < w; j++)
            {
                p->g = (p->g * anti_alpha + g_scaled) >> 8;
                p->r = (p->r * anti_alpha + r_scaled) >> 8;
                p->b = (p->b * anti_alpha + b_scaled) >> 8;
                p++;
            }
        }
    }

    // draw a 3x5 char at the specified position
    // x and y are considered to be the top left corner of the character
    void draw_3x5_char(const char ch, const int x, int y, const ws2812::led_color_t c)
    {
        const uint8_t *font_char = font_3x5_missing_char;
        int font_char_column = 0;

        if (ch >= 32 && ch <= 96)
        {
            font_char_column = ch - 32;
            font_char = font_3x5_32_96_optimized[font_char_column / 2];
        }
        else
        {
            if (ch >= 'a' && ch <= 'z') // a-z
            {
                font_char_column = ch - 'a' + 'A' - 32;
                font_char = font_3x5_32_96_optimized[font_char_column / 2];
            }
            else
            {
                if (ch >= 123 && ch <= 126)
                {
                    font_char_column = ch - 122;
                    font_char = font_3x5_122_126_optimized[font_char_column / 2];
                }
            }
        }

        for (int row = 0; row < 5; row++, y++)
        {
            const uint8_t line = (font_char_column % 2) ? font_char[row] : (font_char[row] >> 3);

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

    enum font_3x5_alignment_t
    {
        FONT_3X5_CENTER = 0,
        FONT_3X5_LEFT = 1,
        FONT_3X5_RIGHT = 2
    };

    void draw_3x5_string(const char *str, const int x, const int y, const ws2812::led_color_t c, const font_3x5_alignment_t alignment = FONT_3X5_LEFT)
    {
        switch (alignment)
        {
        case FONT_3X5_CENTER:
            draw_3x5_string(str, x - strlen(str) * 4 / 2, y, c, FONT_3X5_LEFT);
            break;

        case FONT_3X5_RIGHT:
            draw_3x5_string(str, x - strlen(str) * 4 + 1, y, c, FONT_3X5_LEFT);
            break;

        default:
            for (int i = 0; str[i] != '\0'; i++)
            {
                draw_3x5_char(str[i], x + i * 4, y, c);
            }
            break;
        }
    }

    void draw_3x5_number(const uint number, const int x, const int y, const ws2812::led_color_t c, const font_3x5_alignment_t alignment = FONT_3X5_LEFT)
    {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%u", number);
        draw_3x5_string(buffer, x, y, c, alignment);
    }

    void draw_orb(const float x_c, const float y_c, const float radius, const ws2812::led_color_t c)
    {
        if (x_c + radius < 0 || x_c - radius >= SCREEN_WIDTH || y_c + radius < 0 || y_c - radius >= SCREEN_HEIGHT)
        {
            return;
        }
        if (radius <= 0)
        {
            (*scr_screen)[(int)y_c][(int)x_c] = c;
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
}