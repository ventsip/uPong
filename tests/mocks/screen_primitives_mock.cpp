#include "screen_primitives_mock.hpp"
#include <cstring>
#include <cstdio>

namespace screen
{
    void draw_3x5_char(const char ch, const int x, int y, const ws2812::led_color_t c)
    {
        // Simplified mock implementation - just draw a 3x5 rectangle
        for (int i = 0; i < 5; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                set_pixel(x + j, y + i, c);
            }
        }
    }

    void draw_3x5_string(const char *str, const int x, const int y, const ws2812::led_color_t c, const font_3x5_alignment_t alignment)
    {
        int start_x = x;
        switch (alignment)
        {
        case FONT_3X5_CENTER:
            start_x = x - strlen(str) * 4 / 2;
            break;
        case FONT_3X5_RIGHT:
            start_x = x - strlen(str) * 4 + 1;
            break;
        default:
            break;
        }

        for (int i = 0; str[i] != '\0'; i++)
        {
            draw_3x5_char(str[i], start_x + i * 4, y, c);
        }
    }

    void draw_3x5_number(const unsigned int number, const int x, const int y, const ws2812::led_color_t c, const font_3x5_alignment_t alignment)
    {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%u", number);
        draw_3x5_string(buffer, x, y, c, alignment);
    }
}