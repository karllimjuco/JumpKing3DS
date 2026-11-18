#pragma once

#include "VectorMath.hpp"
#include "AdvCollisionInfo.hpp"
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <array>

namespace JumpKing {

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    bool intersects(const Rect& other) const {
        return (x < other.x + other.width &&
                x + width > other.x &&
                y < other.y + other.height &&
                y + height > other.y);
    }
};

struct InputState {
    bool left = false;
    bool right = false;
    bool jump = false;

    int get_dpad_x() const {
        int dir = 0;
        if (right) dir += 1;
        if (left) dir -= 1;
        return dir;
    }
};

template <std::size_t N>
class InputBuffer {
public:
    void reset() {
        m_count = 0;
        m_head = 0;
    }

    void push(const InputState& state) {
        m_items[m_head] = state;
        m_head = (m_head + 1) % N;
        if (m_count < N) {
            m_count++;
        }
    }

    std::size_t count() const {
        return m_count;
    }

    InputState get_item(std::size_t index) const {
        if (index >= m_count) return {};
        std::size_t idx = (m_head + N - 1 - index) % N;
        return m_items[idx];
    }

private:
    std::array<InputState, N> m_items{};
    std::size_t m_count = 0;
    std::size_t m_head = 0;
};

inline float move_towards(float current, float target, float step) {
    float diff = target - current;
    return current + std::clamp(diff, -step, step);
}

class JumpPhysics {
public:
    // Original decompiled PlayerValues (60 FPS fixed step)
    static constexpr int FPS = 60;
    static constexpr float DELTA_TIME = 1.0f / 60.0f;
    static constexpr int PLAYER_WIDTH = 18;
    static constexpr int PLAYER_HEIGHT = 26;
    static constexpr float FPS_SCALE = 0.5f;

    static constexpr float SPEED = 7.0f * FPS_SCALE;               // 3.5f
    static constexpr float WALK_SPEED = 3.0f * FPS_SCALE;          // 1.5f
    static constexpr float BOUNCE = 0.5f;
    static constexpr float MAX_FALL = 20.0f * FPS_SCALE;           // 10.0f
    static constexpr float MAX_SLIDE = 15.0f * FPS_SCALE;          // 7.5f
    static constexpr float ICE_FRICTION = 0.2f * FPS_SCALE;        // 0.1f
    static constexpr float JUMP_TIME = 0.6f;
    static constexpr float SPLAT_TIME = 0.5f;

    // Derived from PlayerValues:
    // JUMP_HEIGHT = SigmaSum1(17) = 153.0f
    // CalculateJumpHeight(34, 1.0f) = SigmaSumf(34, 1, 1.0f) = 595.0f
    // GRAVITY = 153.0f / 595.0f (~0.257142857f)
    // JUMP_STRENGTH = -(34 * 153.0f) / 595.0f (~ -8.742857143f)
    static constexpr float JUMP_HEIGHT = 153.0f;
    static constexpr float GRAVITY = 153.0f / 595.0f;
    static constexpr float JUMP_STRENGTH = -(34.0f * 153.0f) / 595.0f;

    enum class State {
        Grounded,
        Charging,
        Airborne,
        Splat
    };

    struct BlockFlags {
        bool is_ice = false;
        bool is_snow = false;
        bool is_water = false;
        bool is_sand = false;
    };

    Vec2 position{240.0f, 200.0f};
    Vec2 velocity{0.0f, 0.0f};
    Vec2 last_velocity{0.0f, 0.0f};
    int facing_direction = 1;

    State current_state = State::Grounded;
    BlockFlags block_flags{};

    float charge_timer = 0.0f;
    float splat_timer = 0.0f;
    bool can_jump = true;
    bool is_on_ground = true;
    bool is_knocked = false;
    bool play_bump_sfx = false;

    InputBuffer<4> input_buffer{};

    Rect get_hitbox() const {
        return {
            std::floor(position.x),
            std::floor(position.y),
            static_cast<float>(PLAYER_WIDTH),
            static_cast<float>(PLAYER_HEIGHT)
        };
    }

    float get_multipliers() const {
        float mult = 1.0f;
        if (block_flags.is_sand) mult *= 0.5f;
        return mult;
    }

    bool try_consume_jump() {
        bool allowed = can_jump;
        can_jump = false;
        return allowed;
    }

    void start_charge() {
        charge_timer = 0.0f;
        if (!block_flags.is_ice) {
            velocity.x = 0.0f;
        }
        current_state = State::Charging;
    }

    void reset_jump() {
        charge_timer = 0.0f;
        input_buffer.reset();
    }

    void do_jump(float intensity, int input_dpad_x) {
        int x = input_dpad_x;
        if (x == 0) {
            for (std::size_t i = 0; i < input_buffer.count(); ++i) {
                int buffered_x = input_buffer.get_item(i).get_dpad_x();
                if (buffered_x != 0) {
                    x = buffered_x;
                    break;
                }
            }
        }

        if (block_flags.is_snow && intensity < 0.3f) {
            if (!(intensity > 0.2f)) {
                reset_jump();
                current_state = State::Grounded;
                return;
            }
            intensity = 0.3f;
        }

        velocity.y = JUMP_STRENGTH * intensity;
        velocity.x += static_cast<float>(x) * SPEED;
        velocity.x = std::clamp(velocity.x, -SPEED, SPEED);

        if (x != 0) {
            facing_direction = x;
        }

        reset_jump();
        is_on_ground = false;
        current_state = State::Airborne;
    }

    void update_walk(int dpad_x) {
        float target_x = static_cast<float>(dpad_x) * WALK_SPEED;

        if (block_flags.is_snow) {
            velocity.x = 0.0f;
        } else if (block_flags.is_ice) {
            bool same_sign = (target_x > 0.0f && velocity.x > 0.0f) ||
                             (target_x < 0.0f && velocity.x < 0.0f);
            if (target_x != 0.0f && (!same_sign || std::abs(target_x) > std::abs(velocity.x))) {
                velocity.x = move_towards(velocity.x, target_x, ICE_FRICTION * 2.0f);
            }
        } else {
            velocity.x = target_x;
        }

        if (dpad_x != 0) {
            facing_direction = dpad_x;
        }
    }

    // ResolveXCollisionBehaviour with slope vector projection
    template <typename CollisionQueryFn>
    void update_and_resolve_x(CollisionQueryFn&& query_collision) {
        position.x += velocity.x;

        Rect hitbox = get_hitbox();
        AdvCollisionInfo info;
        if (query_collision(hitbox, info)) {
            position.x = std::floor(position.x);
            int step = (velocity.x > 0.0f) ? 1 : -1;

            AdvCollisionInfo step_info = info;
            do {
                info = step_info;
                position.x -= static_cast<float>(step);
                hitbox = get_hitbox();
            } while (query_collision(hitbox, step_info));

            if (info.slope_type == SlopeType::None) {
                velocity.x *= -BOUNCE;
                is_knocked = true;
                if (!is_on_ground || velocity.y <= 0.0f) {
                    play_bump_sfx = true;
                }
            } else {
                if (is_on_ground && std::abs(velocity.x) <= WALK_SPEED && velocity.y > 0.0f) {
                    velocity.x = 0.0f;
                } else if (VectorMath::dot(last_velocity, info.slope_normal) < 0.0f) {
                    // Project velocity along the slope tangent
                    velocity = VectorMath::make_composite(velocity, info.slope_normal).i;
                }
            }
        }
    }

    // ResolveYCollisionBehaviour with slope landing & ceiling deflection
    template <typename CollisionQueryFn>
    void update_and_resolve_y(CollisionQueryFn&& query_collision) {
        position.y += velocity.y;
        int y_step = (velocity.y > 0.0f) ? 1 : -1;

        Rect hitbox = get_hitbox();
        AdvCollisionInfo info;
        if (query_collision(hitbox, info)) {
            position.y = std::floor(position.y);
            AdvCollisionInfo step_info = info;
            do {
                info = step_info;
                position.y -= static_cast<float>(y_step);
                hitbox = get_hitbox();
            } while (query_collision(hitbox, step_info));

            if (y_step > 0) {
                // Moving downwards (landing)
                bool is_slope = (info.slope_type != SlopeType::None);
                is_on_ground = true;

                if (is_slope) {
                    is_on_ground = false;
                    if (VectorMath::dot(last_velocity, info.slope_normal) < 0.0f) {
                        // Project along slope tangent and set knocked
                        velocity = VectorMath::make_composite(velocity, info.slope_normal).i;
                        is_knocked = true;
                    } else {
                        is_slope = false;
                    }
                }

                if (!is_slope) {
                    is_knocked = false;
                    if (last_velocity.y >= MAX_FALL) {
                        current_state = State::Splat;
                        splat_timer = 0.0f;
                        velocity.x = 0.0f;
                    } else {
                        current_state = State::Grounded;
                    }
                }
            } else {
                // Moving upwards (ceiling hit)
                play_bump_sfx = true;
                if (info.slope_type == SlopeType::None) {
                    velocity.x *= BOUNCE;
                } else if (VectorMath::dot(last_velocity, info.slope_normal) < 0.0f) {
                    UnitVectorPair2D pair = VectorMath::make_composite(velocity, info.slope_normal);
                    velocity.y = pair.i.y;
                    velocity.x *= BOUNCE;
                }
            }

            if (info.slope_type == SlopeType::None || velocity.y <= 0.0f || is_on_ground) {
                velocity.y = 0.0f;
            }
        } else {
            is_on_ground = false;
            if (current_state == State::Grounded) {
                current_state = State::Airborne;
            }
        }
    }

    template <typename CollisionQueryFn>
    void tick(const InputState& input, const InputState& pressed_input, CollisionQueryFn&& query_collision) {
        last_velocity = velocity;
        play_bump_sfx = false;

        if (pressed_input.jump) can_jump = true;
        if (!input.jump) can_jump = false;

        switch (current_state) {
            case State::Grounded:
                if (pressed_input.jump && try_consume_jump()) {
                    input_buffer.push(input);
                    start_charge();
                } else {
                    update_walk(input.get_dpad_x());
                }
                break;
            case State::Charging:
                input_buffer.push(input);
                if (!block_flags.is_ice) velocity.x = 0.0f;
                charge_timer += DELTA_TIME * get_multipliers();
                if (charge_timer >= JUMP_TIME || !input.jump) {
                    float intensity = std::min(1.0f, charge_timer / JUMP_TIME);
                    do_jump(intensity, input.get_dpad_x());
                }
                break;
            case State::Airborne:
                break;
            case State::Splat:
                if (!block_flags.is_ice) velocity.x = 0.0f;
                splat_timer += DELTA_TIME;
                if (splat_timer >= SPLAT_TIME && (input.left || input.right || input.jump)) {
                    current_state = State::Grounded;
                    is_on_ground = true;
                }
                break;
        }

        // 1. Update and resolve X
        if (velocity.x != 0.0f) {
            update_and_resolve_x(query_collision);
        }

        // 2. Update and resolve Y
        update_and_resolve_y(query_collision);

        // 3. Gravity
        if (current_state == State::Airborne) {
            velocity.y = std::min(velocity.y + GRAVITY, MAX_FALL);
        }

        // 4. Block friction (Ice)
        if (block_flags.is_ice && is_on_ground) {
            velocity.x = move_towards(velocity.x, 0.0f, ICE_FRICTION);
        }
    }
};

} // namespace JumpKing
