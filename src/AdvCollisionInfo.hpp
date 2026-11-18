#pragma once

#include "VectorMath.hpp"

namespace JumpKing {

enum class SlopeType {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    None
};

inline Vec2 get_slope_normal(SlopeType type) {
    constexpr float INV_SQRT2 = 0.7071067811865475f;
    switch (type) {
        case SlopeType.TopLeft:     return {-INV_SQRT2, -INV_SQRT2};
        case SlopeType.TopRight:    return { INV_SQRT2, -INV_SQRT2};
        case SlopeType.BottomLeft:  return {-INV_SQRT2,  INV_SQRT2};
        case SlopeType.BottomRight: return { INV_SQRT2,  INV_SQRT2};
        case SlopeType.None:
        default:                    return {0.0f, 0.0f};
    }
}

struct AdvCollisionInfo {
    bool has_collision = false;
    SlopeType slope_type = SlopeType::None;
    Vec2 slope_normal{0.0f, 0.0f};

    bool ice = false;
    bool snow = false;
    bool water = false;
    bool sand = false;
    bool quark = false;

    AdvCollisionInfo() = default;

    AdvCollisionInfo(SlopeType type, bool is_ice = false, bool is_snow = false)
        : has_collision(true), slope_type(type), slope_normal(get_slope_normal(type)), ice(is_ice), snow(is_snow) {}
};

} // namespace JumpKing
