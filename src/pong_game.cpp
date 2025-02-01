#include "pong_game.hpp"
#include "rotary_encoder.hpp"
#include "screen_primitives.hpp"

namespace pong_game
{
    static const ws2812::led_color_t COLOR_WHITE = {0, 255, 255, 255};

    static float ball_x = screen::SCREEN_WIDTH / 2;
    static float ball_y = screen::SCREEN_HEIGHT / 2;

    const static float ball_speed_x = 20; // pixels per second
    const static float ball_speed_y = 20; // pixels per second
    static float ball_dir_x = 1;
    static float ball_dir_y = 0.3;

    static float left_paddle_y = screen::SCREEN_HEIGHT / 2;
    static float right_paddle_y = screen::SCREEN_HEIGHT / 2;

    static float paddle_speed = .25; // pixels per click

    void game_init()
    {
        screen::scr_clear_screen();
    }

    void game_update(const absolute_time_t /*current_time*/, const absolute_time_t delta_time_us)
    {
        const float delta_time_s = (float)delta_time_us / 1000000.0;

        int32_t rotary_1_delta = rotary_encoder::rotary_encoder_fetch_counter(&rotary_encoder::rotary_encoders[0]);
        left_paddle_y -= rotary_1_delta * paddle_speed;
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
        right_paddle_y -= rotary_2_delta * paddle_speed;
        // sw_2_state = rotary_encoder::rotary_encoder_fetch_sw_state(&rotary_encoder::rotary_encoders[1]);
        if (right_paddle_y < 0)
        {
            right_paddle_y = 0;
        }
        if (right_paddle_y >= screen::SCREEN_HEIGHT)
        {
            right_paddle_y = screen::SCREEN_HEIGHT - 1;
        }

        ball_x += ball_speed_x * delta_time_s * ball_dir_x;
        ball_y += ball_speed_y * delta_time_s * ball_dir_y;

        if (ball_x < 0)
        {
            ball_x = 0;
            ball_dir_x = -ball_dir_x;
        }
        if (ball_x >= screen::SCREEN_WIDTH)
        {
            ball_x = screen::SCREEN_WIDTH - 1;
            ball_dir_x = -ball_dir_x;
        }

        if (ball_y < 0)
        {
            ball_y = 0;
            ball_dir_y = -ball_dir_y;
        }
        if (ball_y >= screen::SCREEN_HEIGHT)
        {
            ball_y = screen::SCREEN_HEIGHT - 1;
            ball_dir_y = -ball_dir_y;
        }
    }

    void game_draw(const bool gamma, const bool dither)
    {
        screen::scr_clear_screen();

        // draw field
        screen::draw_horizontal_line(0, 0, screen::SCREEN_WIDTH - 1, COLOR_WHITE);
        screen::draw_horizontal_line(screen::SCREEN_HEIGHT - 1, 0, screen::SCREEN_WIDTH - 1, COLOR_WHITE);
        screen::draw_vertical_line(screen::SCREEN_WIDTH / 2, 0, screen::SCREEN_HEIGHT - 1, COLOR_WHITE);

        // draw a ball
        screen::set_pixel(ball_x, ball_y, COLOR_WHITE);

        // draw left paddle
        screen::draw_vertical_line(0, left_paddle_y - 1, left_paddle_y + 1, COLOR_WHITE);
        // draw right paddle
        screen::draw_vertical_line(screen::SCREEN_WIDTH - 1, right_paddle_y - 1, right_paddle_y + 1, COLOR_WHITE);

        screen::scr_screen_swap(gamma, dither);
    }

    void game_exit()
    {
    }
} // namespace pong_game
