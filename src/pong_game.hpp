#pragma once
#include <pico/time.h> // Add this line to include the definition of absolute_time_t

namespace pong_game
{
    void game_init();
    void game_update(const absolute_time_t current_time, const absolute_time_t delta_time_us);
    void game_draw();
    void game_exit();
}
