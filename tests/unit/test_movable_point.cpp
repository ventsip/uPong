#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../game_logic.hpp"

using namespace pong_game;

TEST_CASE("CMovablePoint physics and movement", "[CMovablePoint]")
{
    SECTION("Construction with initial position and velocity")
    {
        CPoint initial_pos(10.0f, 20.0f);
        CVector velocity(5.0f, -3.0f);
        
        CMovablePoint mp(initial_pos, velocity);
        
        REQUIRE(mp.pos_now.x == Catch::Approx(10.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(20.0f));
        REQUIRE(mp.pos_prev.x == Catch::Approx(10.0f));
        REQUIRE(mp.pos_prev.y == Catch::Approx(20.0f));
        REQUIRE(mp.vel.x == Catch::Approx(5.0f));
        REQUIRE(mp.vel.y == Catch::Approx(-3.0f));
    }

    SECTION("Basic movement with positive velocity")
    {
        CMovablePoint mp(CPoint(0.0f, 0.0f), CVector(10.0f, 5.0f));
        
        mp.update(1.0f); // 1 second
        
        REQUIRE(mp.pos_now.x == Catch::Approx(10.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(5.0f));
        REQUIRE(mp.pos_prev.x == Catch::Approx(0.0f));
        REQUIRE(mp.pos_prev.y == Catch::Approx(0.0f));
    }

    SECTION("Movement with negative velocity")
    {
        CMovablePoint mp(CPoint(10.0f, 10.0f), CVector(-5.0f, -2.0f));
        
        mp.update(2.0f); // 2 seconds
        
        REQUIRE(mp.pos_now.x == Catch::Approx(0.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(6.0f));
        REQUIRE(mp.pos_prev.x == Catch::Approx(10.0f));
        REQUIRE(mp.pos_prev.y == Catch::Approx(10.0f));
    }

    SECTION("Movement with fractional time step")
    {
        CMovablePoint mp(CPoint(0.0f, 0.0f), CVector(20.0f, 30.0f));
        
        mp.update(0.5f); // Half second
        
        REQUIRE(mp.pos_now.x == Catch::Approx(10.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(15.0f));
    }

    SECTION("Multiple consecutive updates")
    {
        CMovablePoint mp(CPoint(0.0f, 0.0f), CVector(1.0f, 2.0f));
        
        // First update
        mp.update(1.0f);
        REQUIRE(mp.pos_now.x == Catch::Approx(1.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(2.0f));
        REQUIRE(mp.pos_prev.x == Catch::Approx(0.0f));
        REQUIRE(mp.pos_prev.y == Catch::Approx(0.0f));
        
        // Second update
        mp.update(1.0f);
        REQUIRE(mp.pos_now.x == Catch::Approx(2.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(4.0f));
        REQUIRE(mp.pos_prev.x == Catch::Approx(1.0f));
        REQUIRE(mp.pos_prev.y == Catch::Approx(2.0f));
        
        // Third update
        mp.update(1.0f);
        REQUIRE(mp.pos_now.x == Catch::Approx(3.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(6.0f));
        REQUIRE(mp.pos_prev.x == Catch::Approx(2.0f));
        REQUIRE(mp.pos_prev.y == Catch::Approx(4.0f));
    }

    SECTION("Zero velocity movement")
    {
        CMovablePoint mp(CPoint(5.0f, 7.0f), CVector(0.0f, 0.0f));
        
        mp.update(10.0f); // Large time step
        
        REQUIRE(mp.pos_now.x == Catch::Approx(5.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(7.0f));
        REQUIRE(mp.pos_prev.x == Catch::Approx(5.0f));
        REQUIRE(mp.pos_prev.y == Catch::Approx(7.0f));
    }

    SECTION("Zero time step")
    {
        CMovablePoint mp(CPoint(3.0f, 4.0f), CVector(100.0f, 200.0f));
        
        mp.update(0.0f); // No time passed
        
        REQUIRE(mp.pos_now.x == Catch::Approx(3.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(4.0f));
        REQUIRE(mp.pos_prev.x == Catch::Approx(3.0f));
        REQUIRE(mp.pos_prev.y == Catch::Approx(4.0f));
    }

    SECTION("Very small time steps (precision test)")
    {
        CMovablePoint mp(CPoint(0.0f, 0.0f), CVector(1000.0f, 1000.0f));
        
        mp.update(0.001f); // 1 millisecond
        
        REQUIRE(mp.pos_now.x == Catch::Approx(1.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(1.0f));
    }

    SECTION("High velocity movement")
    {
        CMovablePoint mp(CPoint(0.0f, 0.0f), CVector(1000.0f, -500.0f));
        
        mp.update(0.1f);
        
        REQUIRE(mp.pos_now.x == Catch::Approx(100.0f));
        REQUIRE(mp.pos_now.y == Catch::Approx(-50.0f));
    }
}