#pragma once

#include <cmath>

namespace JumpKing {

struct UnitVectorPair2D {
    Vec2 i; // Tangential component (along slope surface)
    Vec2 j; // Normal component (into/out of slope)
};

class VectorMath {
public:
    static constexpr float dot(const Vec2& a, const Vec2& b) {
        return a.x * b.x + a.y * b.y;
    }

    static float magnitude(const Vec2& vec) {
        return std::sqrt(vec.x * vec.x + vec.y * vec.y);
    }

    static Vec2 normalize(const Vec2& vec) {
        float mag = magnitude(vec);
        if (mag <= 0.0f) {
            return {0.0f, 0.0f};
        }
        return {vec.x / mag, vec.y / mag};
    }

    // Perpendicular vector: (y, -x) as in original decompiled ErikMaths.VectorMath
    static constexpr Vec2 perpendicular(const Vec2& vec) {
        return {vec.y, -vec.x};
    }

    // Decomposes vector into tangential (i) and normal (j) projections relative to axis_j
    static UnitVectorPair2D make_composite(const Vec2& vec, Vec2 axis_j) {
        axis_j = normalize(axis_j);
        Vec2 vector = perpendicular(axis_j);

        float dot_i = dot(vec, vector);
        float dot_j = dot(vec, axis_j);

        return UnitVectorPair2D{
            .i = {dot_i * vector.x, dot_i * vector.y},
            .j = {dot_j * axis_j.x, dot_j * axis_j.y}
        };
    }
};

} // namespace JumpKing
