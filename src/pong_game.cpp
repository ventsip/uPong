#include "pong_game.hpp"
#include "rotary_encoder.hpp"
#include "screen_primitives.hpp"

namespace pong_game
{
    const ws2812::led_color_t COLOR_WHITE = {0, 255, 255, 255};

    int ball_x = screen::SCREEN_WIDTH / 2;
    int ball_y = screen::SCREEN_HEIGHT / 2;

    int ball_speed_x = 1;
    int ball_speed_y = 1;

    int ball_speed_x_counter = 0;
    int ball_speed_y_counter = 0;

    int left_paddle_y = screen::SCREEN_HEIGHT / 2;
    int right_paddle_y = screen::SCREEN_HEIGHT / 2;

    void game_init()
    {
        screen::scr_clear_screen();
    }

    void game_update()
    {
        int32_t rotary_1_delta = rotary_encoder::rotary_encoder_fetch_counter(&rotary_encoder::rotary_encoders[0]);
        left_paddle_y -= rotary_1_delta;
        // sw_1_state = rotary_encoder::rotary_encoder_fetch_sw_state(&rotary_encoder::rotary_encoders[0]);
        if (left_paddle_y < 0)
        {
            left_paddle_y = 0;
        }
        if (left_paddle_y >= screen::SCREEN_HEIGHT)
        {
            left_paddle_y = screen::SCREEN_HEIGHT - 1;
        }

        int32_t rotary_2_delta = rotary_encoder::rotary_encoder_fetch_counter(&rotary_encoder::rotary_encoders[1]);
        right_paddle_y -= rotary_2_delta;
        // sw_2_state = rotary_encoder::rotary_encoder_fetch_sw_state(&rotary_encoder::rotary_encoders[1]);
        if (right_paddle_y < 0)
        {
            right_paddle_y = 0;
        }
        if (right_paddle_y >= screen::SCREEN_HEIGHT)
        {
            right_paddle_y = screen::SCREEN_HEIGHT - 1;
        }
    }

    void game_draw()
    {
        screen::scr_clear_screen();

        // draw field
        screen::draw_line(0, 0, screen::SCREEN_WIDTH - 1, 0, COLOR_WHITE);
        screen::draw_line(0, screen::SCREEN_HEIGHT - 1, screen::SCREEN_WIDTH - 1, screen::SCREEN_HEIGHT - 1, COLOR_WHITE);
        // draw a vertical line in the middle of the screen
        screen::draw_line(screen::SCREEN_WIDTH / 2, 0, screen::SCREEN_WIDTH / 2, screen::SCREEN_HEIGHT - 1, COLOR_WHITE);
        // draw a ball
        screen::set_pixel(ball_x, ball_y, COLOR_WHITE);

        // draw left paddle
        screen::draw_vertical_line(0, left_paddle_y - 1, left_paddle_y + 1, COLOR_WHITE);
        // draw right paddle
        screen::draw_vertical_line(screen::SCREEN_WIDTH - 1, right_paddle_y - 1, right_paddle_y + 1, COLOR_WHITE);

        screen::scr_screen_swap(true, true);
    }

    void game_exit()
    {
    }
} // namespace pong_game
