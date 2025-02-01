#include "pong_game.hpp"
#include "rotary_encoder.hpp"
#include "screen_primitives.hpp"

namespace pong_game
{
    class CPoint
    {
    public:
        float x;
        float y;
        CPoint(float x, float y) : x(x), y(y) {}
    };

    typedef CPoint CVector;

    class CField
    {
    private:
        CPoint position;

        CPoint size;
        ws2812::led_color_t color;

    public:
        CField(float x, float y, float width, float height, ws2812::led_color_t color) : position(x, y), size(width, height), color(color) {}

        void draw()
        {
            screen::draw_horizontal_line(position.y, position.x, position.x + size.x, color);
            screen::draw_horizontal_line(position.y + size.y, position.x, position.x + size.x, color);
            screen::draw_vertical_line(position.x + size.x / 2, position.y, position.y + size.y, color);
        }

        const CPoint &getSize() const
        {
            return size;
        }

        const CPoint &getPosition() const
        {
            return position;
        }
    };

    class CBall
    {
    private:
        CPoint position;
        ws2812::led_color_t color;
        CVector velocity;

        const CField &field;

    public:
        CBall(const CPoint &pos, const CVector &vel, ws2812::led_color_t color, const CField &field) : position(pos), color(color), velocity(vel), field(field) {}

        void draw()
        {
            screen::set_pixel(position.x, position.y, color);
        }

        void update(const float delta_time_s)
        {
            position.x += velocity.x * delta_time_s;
            position.y += velocity.y * delta_time_s;

            if (position.x < field.getPosition().x)
            {
                position.x = field.getPosition().x;
                velocity.x = -velocity.x;
            }
            if (position.x > field.getPosition().x + field.getSize().x)
            {
                position.x = field.getPosition().x + field.getSize().x;
                velocity.x = -velocity.x;
            }
            if (position.y < field.getPosition().y)
            {
                position.y = field.getPosition().y;
                velocity.y = -velocity.y;
            }
            if (position.y > field.getPosition().y + field.getSize().y)
            {
                position.y = field.getPosition().y + field.getSize().y;
                velocity.y = -velocity.y;
            }
        }
    };

    class CPaddle
    {
    private:
        CPoint position;
        ws2812::led_color_t color;
        const CField &field;

    public:
        CPaddle(const CPoint &pos, ws2812::led_color_t color, const CField &field) : position(pos), color(color), field(field) {}

        void draw()
        {
            screen::draw_vertical_line(position.x, position.y - 1, position.y + 1, color);
        }

        void move(const float delta)
        {
            position.y += delta;
            if (position.y < field.getPosition().y)
            {
                position.y = field.getPosition().y;
            }
            if (position.y > field.getPosition().y + field.getSize().y)
            {
                position.y = field.getPosition().y + field.getSize().y;
            }
        }
    };

    static const ws2812::led_color_t COLOR_FIELD_LINE = ws2812_pack_color(255, 255, 128);
    static const ws2812::led_color_t COLOR_BALL = ws2812_pack_color(255, 255, 64);
    static const ws2812::led_color_t COLOR_PADDLE = ws2812_pack_color(255, 255, 255);

    static float paddle_speed = .25; // pixels per click

    void game_init()
    {
        screen::scr_clear_screen();
    }

    static CField field(0, 0, screen::SCREEN_WIDTH - 1, screen::SCREEN_HEIGHT - 1, COLOR_FIELD_LINE);
    static CBall ball(CPoint(field.getSize().x / 2, field.getSize().y / 2), CVector(20, 16), COLOR_BALL, field);
    static CPaddle left_paddle(CPoint(field.getPosition().x, field.getPosition().y + field.getSize().y / 2), COLOR_PADDLE, field);
    static CPaddle right_paddle(CPoint(field.getPosition().x + field.getSize().x, field.getPosition().y + field.getSize().y / 2), COLOR_PADDLE, field);

    void game_update(const absolute_time_t /*current_time*/, const absolute_time_t delta_time_us)
    {
        const float delta_time_s = (float)delta_time_us / 1000000.0;

        int32_t rotary_1_delta = rotary_encoder::rotary_encoder_fetch_counter(&rotary_encoder::rotary_encoders[0]);
        // sw_1_state = rotary_encoder::rotary_encoder_fetch_sw_state(&rotary_encoder::rotary_encoders[0]);
        left_paddle.move(rotary_1_delta * paddle_speed);

        int32_t rotary_2_delta = rotary_encoder::rotary_encoder_fetch_counter(&rotary_encoder::rotary_encoders[1]);
        // sw_2_state = rotary_encoder::rotary_encoder_fetch_sw_state(&rotary_encoder::rotary_encoders[1]);
        right_paddle.move(rotary_2_delta * paddle_speed);

        ball.update(delta_time_s);
    }

    void game_draw(const bool gamma, const bool dither)
    {
        screen::scr_clear_screen();

        // draw field
        field.draw();

        // draw a ball
        ball.draw();

        // draw paddles
        left_paddle.draw();
        right_paddle.draw();

        screen::scr_screen_swap(gamma, dither);
    }

    void game_exit()
    {
    }
} // namespace pong_game
