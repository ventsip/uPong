#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../game_logic.hpp"

using namespace pong_game;

TEST_CASE("Paddle collision detection", "[collision]")
{
    // Setup field and colors for testing
    const ws2812::led_color_t white = ws2812_pack_color(255, 255, 255);
    const ws2812::led_color_t red = ws2812_pack_color(255, 0, 0);
    const ws2812::led_color_t blue = ws2812_pack_color(0, 0, 255);
    
    CField field(0, 0, 48, 32, white, red, blue);

    SECTION("Left paddle collision from right side")
    {
        // Setup left paddle at x=0, center of field
        CPaddle left_paddle(CPoint(0, 16), white, field);
        
        // Ball moving left, should hit paddle
        CBall ball(CPoint(0.5f, 16), 1.0f, CVector(-10, 0), white);
        ball.pos_prev = CPoint(2, 16); // Previous position was further right
        
        bool collision = check_paddle_collision(ball, left_paddle, left_paddle.pos_prev.x + 1, true);
        
        REQUIRE(collision == true);
        REQUIRE(ball.vel.x > 0); // Velocity should reverse to positive (moving right)
    }

    SECTION("Right paddle collision from left side")
    {
        // Setup right paddle at right edge
        CPaddle right_paddle(CPoint(47, 16), white, field);
        
        // Ball moving right, should hit paddle
        CBall ball(CPoint(46.5f, 16), 1.0f, CVector(10, 0), white);
        ball.pos_prev = CPoint(45, 16); // Previous position was further left
        
        bool collision = check_paddle_collision(ball, right_paddle, right_paddle.pos_prev.x - 1, false);
        
        REQUIRE(collision == true);
        REQUIRE(ball.vel.x < 0); // Velocity should reverse to negative (moving left)
    }

    SECTION("Ball misses paddle vertically")
    {
        CPaddle left_paddle(CPoint(0, 16), white, field);
        
        // Ball passes above paddle (paddle extends from y=14 to y=18, ball at y=20)
        CBall ball(CPoint(0, 20), 1.0f, CVector(-10, 0), white);
        ball.pos_prev = CPoint(3, 20);
        
        bool collision = check_paddle_collision(ball, left_paddle, left_paddle.pos_prev.x + 1, true);
        
        REQUIRE(collision == false);
        REQUIRE(ball.vel.x == Catch::Approx(-10)); // Velocity unchanged
    }

    SECTION("Ball doesn't cross paddle threshold")
    {
        CPaddle left_paddle(CPoint(0, 16), white, field);
        
        // Ball hasn't crossed the paddle yet but trajectory crosses
        CBall ball(CPoint(1.5f, 16), 1.0f, CVector(-10, 0), white);
        ball.pos_prev = CPoint(3, 16); // Previous position was further right
        
        bool collision = check_paddle_collision(ball, left_paddle, left_paddle.pos_prev.x + 1, true);
        
        REQUIRE(collision == false); // Ball hasn't actually reached paddle collision zone yet
    }

    SECTION("Ball angle changes based on hit position")
    {
        CPaddle left_paddle(CPoint(0, 16), white, field);
        
        // Ball hits top part of paddle (offset_y = -1)
        CBall ball(CPoint(0, 15), 1.0f, CVector(-10, 0), white);
        ball.pos_prev = CPoint(3, 15);
        
        float original_vel_y = ball.vel.y;
        check_paddle_collision(ball, left_paddle, left_paddle.pos_prev.x + 1, true);
        
        // Ball should have gained some upward velocity due to rotation
        REQUIRE(ball.vel.y != Catch::Approx(original_vel_y));
        REQUIRE(ball.vel.x > 0); // Still reversed
    }

    SECTION("Ball hits bottom part of paddle")
    {
        CPaddle left_paddle(CPoint(0, 16), white, field);
        
        // Ball hits bottom part of paddle (offset_y = +1)
        CBall ball(CPoint(0, 17), 1.0f, CVector(-10, 0), white);
        ball.pos_prev = CPoint(3, 17);
        
        float original_vel_y = ball.vel.y;
        check_paddle_collision(ball, left_paddle, left_paddle.pos_prev.x + 1, true);
        
        // Ball should have gained some downward velocity due to rotation
        REQUIRE(ball.vel.y != Catch::Approx(original_vel_y));
        REQUIRE(ball.vel.x > 0); // Still reversed
    }

    SECTION("Ball hits paddle center - minimal angle change")
    {
        CPaddle left_paddle(CPoint(0, 16), white, field);
        
        // Ball hits center of paddle (offset_y = 0)
        CBall ball(CPoint(0, 16), 1.0f, CVector(-10, 0), white);
        ball.pos_prev = CPoint(3, 16);
        
        check_paddle_collision(ball, left_paddle, left_paddle.pos_prev.x + 1, true);
        
        // Velocity should reverse but angle should be minimal since rotation = 0
        REQUIRE(ball.vel.x > 0);
        REQUIRE(ball.vel.y == Catch::Approx(0).margin(1e-6));
    }

    SECTION("Ball moving away from paddle - no collision")
    {
        CPaddle left_paddle(CPoint(0, 16), white, field);
        
        // Ball moving away from paddle
        CBall ball(CPoint(2, 16), 1.0f, CVector(10, 0), white); // Moving right
        ball.pos_prev = CPoint(1, 16);
        
        bool collision = check_paddle_collision(ball, left_paddle, left_paddle.pos_prev.x + 1, true);
        
        REQUIRE(collision == false);
        REQUIRE(ball.vel.x == Catch::Approx(10)); // Velocity unchanged
    }

    SECTION("Velocity direction validation prevents incorrect bounces")
    {
        CPaddle left_paddle(CPoint(0, 16), white, field);
        
        // Simulate case where ball velocity would be wrong direction after rotation
        CBall ball(CPoint(0, 14), 1.0f, CVector(-5, 0), white); // Large offset might cause issues
        ball.pos_prev = CPoint(3, 14);
        
        CVector original_vel = ball.vel;
        bool collision = check_paddle_collision(ball, left_paddle, left_paddle.pos_prev.x + 1, true);
        
        if (collision) {
            // If collision occurred, ball should be moving away from paddle
            REQUIRE(ball.vel.x > 0);
        }
    }
}