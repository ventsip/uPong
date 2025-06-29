#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "../game_logic.hpp"
#include <cmath>

using namespace pong_game;

TEST_CASE("CPoint basic operations", "[CPoint]")
{
    SECTION("Construction and member access")
    {
        CPoint p(3.5f, 2.1f);
        REQUIRE(p.x == Catch::Approx(3.5f));
        REQUIRE(p.y == Catch::Approx(2.1f));
    }

    SECTION("Addition assignment operator")
    {
        CPoint p1(1.0f, 2.0f);
        CPoint p2(3.0f, 4.0f);
        
        p1 += p2;
        
        REQUIRE(p1.x == Catch::Approx(4.0f));
        REQUIRE(p1.y == Catch::Approx(6.0f));
        
        // Original p2 should be unchanged
        REQUIRE(p2.x == Catch::Approx(3.0f));
        REQUIRE(p2.y == Catch::Approx(4.0f));
    }

    SECTION("Scalar multiplication operator")
    {
        CPoint p(2.0f, 3.0f);
        
        CPoint result = p * 2.5f;
        
        REQUIRE(result.x == Catch::Approx(5.0f));
        REQUIRE(result.y == Catch::Approx(7.5f));
        
        // Original point should be unchanged
        REQUIRE(p.x == Catch::Approx(2.0f));
        REQUIRE(p.y == Catch::Approx(3.0f));
    }

    SECTION("Zero and negative scalar multiplication")
    {
        CPoint p(4.0f, -2.0f);
        
        CPoint zero_result = p * 0.0f;
        REQUIRE(zero_result.x == Catch::Approx(0.0f));
        REQUIRE(zero_result.y == Catch::Approx(0.0f));
        
        CPoint neg_result = p * -1.5f;
        REQUIRE(neg_result.x == Catch::Approx(-6.0f));
        REQUIRE(neg_result.y == Catch::Approx(3.0f));
    }
}

TEST_CASE("CVector rotation operations", "[CVector]")
{
    SECTION("Construction from CPoint base class")
    {
        CVector v(1.0f, 0.0f);
        REQUIRE(v.x == Catch::Approx(1.0f));
        REQUIRE(v.y == Catch::Approx(0.0f));
    }

    SECTION("90 degree rotation")
    {
        CVector v(1.0f, 0.0f);
        v.rotate(M_PI / 2.0f); // 90 degrees
        
        REQUIRE(v.x == Catch::Approx(0.0f).margin(1e-6));
        REQUIRE(v.y == Catch::Approx(1.0f));
    }

    SECTION("180 degree rotation")
    {
        CVector v(1.0f, 0.0f);
        v.rotate(M_PI); // 180 degrees
        
        REQUIRE(v.x == Catch::Approx(-1.0f));
        REQUIRE(v.y == Catch::Approx(0.0f).margin(1e-6));
    }

    SECTION("270 degree rotation")
    {
        CVector v(1.0f, 0.0f);
        v.rotate(3.0f * M_PI / 2.0f); // 270 degrees
        
        REQUIRE(v.x == Catch::Approx(0.0f).margin(1e-6));
        REQUIRE(v.y == Catch::Approx(-1.0f));
    }

    SECTION("360 degree rotation (full circle)")
    {
        CVector v(3.0f, 4.0f);
        v.rotate(2.0f * M_PI); // 360 degrees
        
        REQUIRE(v.x == Catch::Approx(3.0f));
        REQUIRE(v.y == Catch::Approx(4.0f));
    }

    SECTION("Small angle rotation")
    {
        CVector v(1.0f, 0.0f);
        const float small_angle = 0.1f; // radians
        v.rotate(small_angle);
        
        REQUIRE(v.x == Catch::Approx(cos(small_angle)));
        REQUIRE(v.y == Catch::Approx(sin(small_angle)));
    }

    SECTION("Negative angle rotation")
    {
        CVector v(0.0f, 1.0f);
        v.rotate(-M_PI / 2.0f); // -90 degrees
        
        REQUIRE(v.x == Catch::Approx(1.0f));
        REQUIRE(v.y == Catch::Approx(0.0f).margin(1e-6));
    }

    SECTION("Chain rotations")
    {
        CVector v(1.0f, 0.0f);
        v.rotate(M_PI / 4.0f).rotate(M_PI / 4.0f); // Two 45-degree rotations = 90 degrees
        
        REQUIRE(v.x == Catch::Approx(0.0f).margin(1e-6));
        REQUIRE(v.y == Catch::Approx(1.0f));
    }

    SECTION("Rotation preserves magnitude")
    {
        CVector v(3.0f, 4.0f);
        const float original_magnitude = sqrt(v.x * v.x + v.y * v.y);
        
        v.rotate(1.234f); // Random angle
        
        const float new_magnitude = sqrt(v.x * v.x + v.y * v.y);
        REQUIRE(new_magnitude == Catch::Approx(original_magnitude));
    }
}