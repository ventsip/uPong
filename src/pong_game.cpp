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

    class CMovablePoint
    {
    public:
        CPoint pos_now, pos_prev;
        CVector vel; // units per second
        CMovablePoint(const CPoint &pos, const CVector &vel) : pos_now(pos), pos_prev(pos), vel(vel) {}
        void update(const float delta_time_s)
        {
            pos_prev = pos_now;
            pos_now.x += vel.x * delta_time_s;
            pos_now.y += vel.y * delta_time_s;
        }
    };

    class CField
    {
    private:
        CPoint position;

        CPoint size;
        ws2812::led_color_t color_lines;
        ws2812::led_color_t color_left_field;
        ws2812::led_color_t color_right_field;

    public:
        CField(float x, float y, float width, float height, ws2812::led_color_t color_lines, ws2812::led_color_t color_left_field, ws2812::led_color_t color_right_field) : position(x, y), size(width, height), color_lines(color_lines), color_left_field(color_left_field), color_right_field(color_right_field) {}

        void draw()
        {
            screen::draw_rect(position.x, position.y, size.x / 2, size.y, color_left_field);
            screen::draw_rect(position.x + size.x / 2, position.y, size.x / 2, size.y, color_right_field);
            screen::draw_horizontal_line(position.y, position.x, position.x + size.x, color_lines);
            screen::draw_horizontal_line(position.y + size.y - 1, position.x, position.x + size.x, color_lines);
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

    class CBall : public CMovablePoint
    {
    private:
        float radius;
        ws2812::led_color_t color;

    public:
        CBall(const CPoint &pos, const float radius, const CVector &vel, const ws2812::led_color_t &color) : CMovablePoint(pos, vel), radius(radius), color(color) {}

        void draw()
        {
            screen::draw_orb(pos_now.x, pos_now.y, radius, color);
        }
    };

    class CPaddle : public CMovablePoint
    {
    private:
        ws2812::led_color_t color;
        const CField &field;

    public:
        CPaddle(const CPoint &pos, ws2812::led_color_t color, const CField &field) : CMovablePoint(pos, CPoint(0, 0)), color(color), field(field) {}

        void draw()
        {
            screen::draw_vertical_line(pos_now.x, pos_now.y - 2, pos_now.y + 2, color);
        }

        void update(const float delta_time_s)
        {
            CMovablePoint::update(delta_time_s);

            if (pos_now.y < field.getPosition().y)
            {
                pos_now.y = field.getPosition().y;
            }
            if (pos_now.y > field.getPosition().y + field.getSize().y)
            {
                pos_now.y = field.getPosition().y + field.getSize().y;
            }
        }
    };

    class CMatch
    {
    private:
        int score[2];
        int max_score;
        CPoint pos;
        ws2812::led_color_t color_score;

    public:
        CMatch(const int max_score, const int x, const int y, const ws2812::led_color_t color_score) : max_score(max_score), pos(x, y), color_score(color_score)
        {
            score[0] = 0;
            score[1] = 0;
        }

        void score_point(const int player)
        {
            score[player]++;
        }

        bool is_over() const
        {
            return score[0] >= max_score || score[1] >= max_score;
        }

        int get_score(const int player) const
        {
            return score[player];
        }

        void draw()
        {
            screen::draw_3x5_number(score[0], pos.x - 1, pos.y, color_score, screen::FONT_3X5_RIGHT);
            screen::draw_3x5_number(score[1], pos.x + 1, pos.y, color_score, screen::FONT_3X5_LEFT);
        }
    };

    const int brightness = 64;
    static const ws2812::led_color_t COLOR_FIELD_LINE = ws2812_pack_color(brightness, brightness, (brightness / 2));
    static const ws2812::led_color_t COLOR_FIELD_LEFT = ws2812_pack_color((brightness / 2), 0, 0);
    static const ws2812::led_color_t COLOR_FIELD_RIGHT = ws2812_pack_color(0, 0, (brightness / 2));
    static const ws2812::led_color_t COLOR_BALL = ws2812_pack_color(brightness, brightness, brightness / 4);
    static const ws2812::led_color_t COLOR_PADDLE = ws2812_pack_color(brightness, brightness, brightness);
    static const ws2812::led_color_t COLOR_SCORE = ws2812_pack_color(brightness, brightness, brightness);

    static float paddle_speed = .25; // pixels per click

    void game_init()
    {
        screen::scr_clear_screen();
    }

    static CField field(0, 0, screen::SCREEN_WIDTH, screen::SCREEN_HEIGHT, COLOR_FIELD_LINE, COLOR_FIELD_LEFT, COLOR_FIELD_RIGHT);
    static CBall ball(CPoint(field.getSize().x / 2, field.getSize().y / 2), 1.5, CVector(20, 0), COLOR_BALL);
    static CPaddle left_paddle(CPoint(field.getPosition().x, field.getPosition().y + field.getSize().y / 2), COLOR_PADDLE, field);
    static CPaddle right_paddle(CPoint(field.getPosition().x + field.getSize().x - 1, field.getPosition().y + field.getSize().y / 2), COLOR_PADDLE, field);
    static CMatch match(5, screen::SCREEN_WIDTH / 2, 2, COLOR_SCORE);

    void game_update(const absolute_time_t /*current_time*/, const absolute_time_t delta_time_us)
    {
        const float delta_time_s = (float)delta_time_us / 1000000.0;

        int32_t rotary_1_delta = rotary_encoder::rotary_encoder_fetch_counter(&rotary_encoder::rotary_encoders[0]);
        // sw_1_state = rotary_encoder::rotary_encoder_fetch_sw_state(&rotary_encoder::rotary_encoders[0]);
        left_paddle.vel.y = rotary_1_delta * paddle_speed;
        left_paddle.update(1);

        int32_t rotary_2_delta = rotary_encoder::rotary_encoder_fetch_counter(&rotary_encoder::rotary_encoders[1]);
        // sw_2_state = rotary_encoder::rotary_encoder_fetch_sw_state(&rotary_encoder::rotary_encoders[1]);
        right_paddle.vel.y = rotary_2_delta * paddle_speed;
        right_paddle.update(1);

        ball.update(delta_time_s);

        // check if ball trajectory intersects with paddles
        if (ball.pos_now.x < left_paddle.pos_now.x + 1 && ball.pos_prev.x >= left_paddle.pos_prev.x + 1)
        {
            const auto offset_y = ball.pos_now.y - left_paddle.pos_now.y;
            if (offset_y >= -2 && offset_y <= 2)
            {
                ball.vel.x = -ball.vel.x;
                ball.vel.y += 3 * offset_y;
            }
        }
        if (ball.pos_now.x > right_paddle.pos_now.x - 1 && ball.pos_prev.x <= right_paddle.pos_prev.x - 1)
        {
            const auto offset_y = ball.pos_now.y - right_paddle.pos_now.y;
            if (offset_y >= -2 && offset_y <= 2)
            {
                ball.vel.x = -ball.vel.x;
                ball.vel.y += 3 * offset_y;
            }
        }

        // bounce ball off top and bottom of the field
        if (ball.pos_now.y < field.getPosition().y)
        {
            ball.pos_now.y = field.getPosition().y;
            ball.vel.y = -ball.vel.y;
        }
        if (ball.pos_now.y > field.getPosition().y + field.getSize().y - 1)
        {
            ball.pos_now.y = field.getPosition().y + field.getSize().y - 1;
            ball.vel.y = -ball.vel.y;
        }

        // check if ball leaves the field
        if (ball.pos_now.x < field.getPosition().x)
        {
            match.score_point(1);
            ball.vel.x = -ball.vel.x;
            ball.pos_prev = ball.pos_now = CPoint(field.getSize().x / 4, field.getSize().y / 2);
        }
        if (ball.pos_now.x >= field.getPosition().x + field.getSize().x)
        {
            match.score_point(0);
            ball.vel.x = -ball.vel.x;
            ball.pos_prev = ball.pos_now = CPoint(3 * field.getSize().x / 4, field.getSize().y / 2);
        }
    }

    void game_draw(const bool gamma, const bool dither)
    {
        screen::scr_clear_screen();

        // draw field
        field.draw();

        // draw score
        match.draw();

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
