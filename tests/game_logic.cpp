#include "game_logic.hpp"

namespace pong_game
{
    bool check_paddle_collision(CBall& ball, const CPaddle& paddle, float /*prev_x*/, bool is_left_paddle) {
        if ((is_left_paddle && ball.pos_now.x < paddle.pos_now.x + 1 && ball.pos_prev.x >= paddle.pos_prev.x + 1) ||
            (!is_left_paddle && ball.pos_now.x > paddle.pos_now.x - 1 && ball.pos_prev.x <= paddle.pos_prev.x - 1)) {
            
            const auto offset_y = ball.pos_now.y - paddle.pos_now.y;
            if (offset_y >= -2 && offset_y <= 2) {
                ball.vel.x = -ball.vel.x;
                const float rotation = offset_y * 5 * 3.14159f / 180.0f;
                ball.vel.rotate(rotation);
                if ((is_left_paddle && ball.vel.x <= 0) || (!is_left_paddle && ball.vel.x >= 0)) {
                    ball.vel.rotate(-rotation);
                }
                return true;
            }
        }
        return false;
    }
}