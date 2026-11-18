// C:\JumpKing3DS\test\test_horizontal.cpp
#include "../src/JumpPhysics.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace JumpKing;
    JumpPhysics player;

    // Solid floor at Y >= 200, solid wall at X >= 300
    auto test_level_collision = [](const Rect& box) {
        bool hit_floor = (box.y + box.height) >= 200.0f;
        bool hit_wall = (box.x + box.width) >= 300.0f;
        return hit_floor || hit_wall;
    };

    // 1. Walking into a wall at X = 300
    player.position = {290.0f, 174.0f}; // Height is 26, rests on floor at 200
    InputState right_input{.right = true};
    InputState no_input{};

    player.tick(right_input, no_input, test_level_collision);
    // Wall bounce reverses horizontal velocity by -BOUNCE (-0.5)
    assert(player.is_knocked == true);
    assert(player.get_hitbox().x + player.get_hitbox().width <= 300.0f);

    // 2. Ice momentum & friction decay test
    player.block_flags.is_ice = true;
    player.velocity.x = 2.0f; // Initial sliding speed
    player.tick(no_input, no_input, test_level_collision);
    // ICE_FRICTION is 0.1f per tick
    assert(std::abs(player.velocity.x - 1.9f) < 1e-5f);

    std::cout << "Horizontal movement & collision tests passed successfully.\n";
    return 0;
}
