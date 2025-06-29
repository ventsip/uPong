#include "ws2812_mock.hpp"

namespace ws2812
{
    // Mock LED color storage
    led_color_t mock_led_colors[NMB_STRIPS][LEDS_PER_STRIP];
    led_color_t (*led_colors)[NMB_STRIPS][LEDS_PER_STRIP] = &mock_led_colors;

    bool WS2812_init()
    {
        clear_led_colors();
        return true;
    }

    void clear_led_colors()
    {
        for (int strip = 0; strip < NMB_STRIPS; strip++)
        {
            for (int led = 0; led < LEDS_PER_STRIP; led++)
            {
                mock_led_colors[strip][led] = ws2812_pack_color(0, 0, 0);
            }
        }
    }

    void transmit_led_colors()
    {
        // Mock implementation - do nothing for host testing
    }
}