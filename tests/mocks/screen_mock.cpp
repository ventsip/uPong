#include "screen_mock.hpp"
#include <cstring>

namespace screen
{
    bool scr_gamma_correction = false;
    bool scr_dither = false;

    // Mock screen buffer
    ws2812::led_color_t mock_screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
    ws2812::led_color_t (*scr_screen)[SCREEN_HEIGHT][SCREEN_WIDTH] = &mock_screen_buffer;

    volatile scr_profile_t scr_profile = {0};

    void scr_screen_init()
    {
        scr_clear_screen();
    }

    void scr_clear_screen()
    {
        memset(mock_screen_buffer, 0, sizeof(mock_screen_buffer));
    }

    void scr_screen_swap(const bool gamma, const bool dither)
    {
        scr_gamma_correction = gamma;
        scr_dither = dither;
        // Mock implementation - do nothing for host testing
    }
}