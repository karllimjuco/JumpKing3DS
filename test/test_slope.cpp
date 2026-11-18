// C:\JumpKing3DS\test\test_slope.cpp
#include "../src/JumpPhysics.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace JumpKing;

    // 1. Verify VectorMath projections
    // Normal for a TopRight 45-degree slope: (1/sqrt(2), -1/sqrt(2))
    Vec2 slope_normal = get_slope_normal(SlopeType::TopRight);
    Vec2 incoming_fall{0.0f, 10.0f}; // Falling straight down

    // Perpendicular vector for (nx, ny) is (ny, -nx)
    // Tangential projection along the slope
    UnitVectorPair2D pair = VectorMath::make_composite(incoming_fall, slope_normal);

    // Falling onto a TopRight slope should yield tangent velocity sliding down & right (x > 0, y > 0)
    assert(pair.i.x > 0.0f);
    assert(pair.i.y > 0.0f);

    // 2. Test Player slope landing interaction
    JumpPhysics player;
    player.position = {200.0f, 100.0f};
    player.velocity = {0.0f, 8.0f};
    player.current_state = JumpPhysics::State::Airborne;

    // Collision callback reporting a TopRight slope hit at Y >= 120
    auto slope_collision = [](const Rect& box, AdvCollisionInfo& info) {
        if (box.y + box.height >= 120.0f) {
            info = AdvCollisionInfo(SlopeType::TopRight);
            return true;
        }
        return false;
    };

    InputState no_input{};
    player.tick(no_input, no_input, slope_collision);

    // Falling onto a slope must not ground the player, must project velocity along tangent, and flag knocked
    assert(!player.is_on_ground);
    assert(player.is_knocked);
    assert(player.velocity.x > 0.0f);

    std::cout << "Slope collision and vector math tests passed successfully.\n";
    return 0;
}
