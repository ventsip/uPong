#pragma once

#include <math.h>

#ifdef HOST_BUILD
#include "ws2812_mock.hpp"
#include "screen_mock.hpp"
#include "screen_primitives_mock.hpp"
#include "rotary_encoder_mock.hpp"
#else
#include "ws2812.hpp"
#include "screen.hpp"
#include "screen_primitives.hpp"
#include "rotary_encoder.hpp"
#endif

// Extract testable game logic classes from pong_game.cpp
namespace pong_game
{
    class CPoint
    {
    public:
        float x;
        float y;
        CPoint(float x, float y) : x(x), y(y) {}

        CPoint &operator+=(const CPoint &other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        CPoint operator*(const float a) const
        {
            return CPoint(x * a, y * a);
        }
    };

    class CVector : public CPoint
    {
    public:
        CVector(const float x, const float y) : CPoint(x, y) {}
        CVector &rotate(const float angle)
        {
            const float sin_angle = sin(angle);
            const float cos_angle = cos(angle);
            const float x_new = x * cos_angle - y * sin_angle;
            const float y_new = x * sin_angle + y * cos_angle;
            x = x_new;
            y = y_new;
            return *this;
        }
    };

    class CMovablePoint
    {
    public:
        CPoint pos_now, pos_prev;
        CVector vel;
        CMovablePoint(const CPoint &pos, const CVector &vel) : pos_now(pos), pos_prev(pos), vel(vel) {}
        void update(const float delta_time_s)
        {
            pos_prev = pos_now;
            pos_now += vel * delta_time_s;
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
        CField(float x, float y, float width, float height, ws2812::led_color_t color_lines, ws2812::led_color_t color_left_field, ws2812::led_color_t color_right_field)
            : position(x, y), size(width, height), color_lines(color_lines), color_left_field(color_left_field), color_right_field(color_right_field) {}

        void draw()
        {
            screen::draw_rect(position.x, position.y, size.x / 2, size.y, color_left_field);
            screen::draw_rect(position.x + size.x / 2, position.y, size.x / 2, size.y, color_right_field);
            screen::draw_horizontal_line(position.y, position.x, position.x + size.x, color_lines);
            screen::draw_horizontal_line(position.y + size.y - 1, position.x, position.x + size.x, color_lines);
        }

        const CPoint &getSize() const { return size; }
        const CPoint &getPosition() const { return position; }
    };

    class CBall : public CMovablePoint
    {
    private:
        float radius;
        ws2812::led_color_t color;

    public:
        CBall(const CPoint &pos, const float radius, const CVector &vel, const ws2812::led_color_t &color)
            : CMovablePoint(pos, vel), radius(radius), color(color) {}

        void draw()
        {
            screen::draw_orb(pos_now.x, pos_now.y, radius, color);
        }

        float getRadius() const { return radius; }
    };

    class CPaddle : public CMovablePoint
    {
    private:
        ws2812::led_color_t color;
        const CField &field;

    public:
        CPaddle(const CPoint &pos, ws2812::led_color_t color, const CField &field)
            : CMovablePoint(pos, CVector(0, 0)), color(color), field(field) {}

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
        CMatch(const int max_score, const int x, const int y, const ws2812::led_color_t color_score)
            : max_score(max_score), pos(x, y), color_score(color_score)
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

    // Collision detection function
    bool check_paddle_collision(CBall& ball, const CPaddle& paddle, float prev_x, bool is_left_paddle);
}