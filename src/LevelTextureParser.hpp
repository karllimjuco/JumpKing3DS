#pragma once

#include "LevelRenderer.hpp"
#include <cstdint>
#include <vector>
#include <cmath>

namespace JumpKing {

struct ColorRGBA {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    constexpr bool equals_rgb(uint8_t pr, uint8_t pg, uint8_t pb, uint8_t tolerance = 15) const {
        return (a >= 128 &&
                std::abs(static_cast<int>(r) - pr) <= tolerance &&
                std::abs(static_cast<int>(g) - pg) <= tolerance &&
                std::abs(static_cast<int>(b) - pb) <= tolerance);
    }
};

using ColorRGB = ColorRGBA; // Backward compatibility

class LevelTextureParser {
public:
    static constexpr int COLS = LevelRenderer::COLS; // 60
    static constexpr int ROWS = LevelRenderer::ROWS; // 45
    static constexpr int TOTAL_TILES = COLS * ROWS;  // 2700

    // Original Jump King level color definitions
    static constexpr ColorRGBA COLOR_SOLID   = {0, 0, 0, 255};       // Black
    static constexpr ColorRGBA COLOR_SLOPE   = {255, 0, 0, 255};     // Red
    static constexpr ColorRGBA COLOR_ICE     = {0, 255, 255, 255};   // Cyan
    static constexpr ColorRGBA COLOR_SNOW    = {255, 255, 0, 255};   // Yellow
    static constexpr ColorRGBA COLOR_SAND    = {255, 106, 0, 255};   // Orange
    static constexpr ColorRGBA COLOR_WATER   = {0, 170, 170, 255};   // Teal
    static constexpr ColorRGBA COLOR_QUARK   = {182, 255, 0, 255};   // Lime

    static bool is_edge(const std::vector<ColorRGBA>& colors, int x, int y) {
        if (x < 0 || x >= COLS || y < 0 || y >= ROWS) {
            return false;
        }
        const ColorRGBA& c = colors[y * COLS + x];
        if (c.a < 128) {
            return true; // Transparent air counts as an edge
        }
        // Non-solid / empty pixel is an edge
        return !c.equals_rgb(COLOR_SOLID.r, COLOR_SOLID.g, COLOR_SOLID.b) &&
               !c.equals_rgb(COLOR_SLOPE.r, COLOR_SLOPE.g, COLOR_SLOPE.b) &&
               !c.equals_rgb(COLOR_ICE.r, COLOR_ICE.g, COLOR_ICE.b) &&
               !c.equals_rgb(COLOR_SNOW.r, COLOR_SNOW.g, COLOR_SNOW.b) &&
               !c.equals_rgb(COLOR_SAND.r, COLOR_SAND.g, COLOR_SAND.b);
    }

    // Parses a 60x45 color buffer into binary tile grid indices
    static std::vector<TileType> parse_color_mask(const std::vector<ColorRGBA>& colors) {
        std::vector<TileType> result(TOTAL_TILES, TileType::Empty);
        if (colors.size() < static_cast<std::size_t>(TOTAL_TILES)) {
            return result;
        }

        for (int y = 0; y < ROWS; ++y) {
            for (int x = 0; x < COLS; ++x) {
                int index = y * COLS + x;
                const ColorRGBA& c = colors[index];

                if (c.a < 128) {
                    result[index] = TileType::Empty;
                } else if (c.equals_rgb(COLOR_SOLID.r, COLOR_SOLID.g, COLOR_SOLID.b)) {
                    result[index] = TileType::Solid;
                } else if (c.equals_rgb(COLOR_ICE.r, COLOR_ICE.g, COLOR_ICE.b)) {
                    result[index] = TileType::Ice;
                } else if (c.equals_rgb(COLOR_SNOW.r, COLOR_SNOW.g, COLOR_SNOW.b)) {
                    result[index] = TileType::Snow;
                } else if (c.equals_rgb(COLOR_SAND.r, COLOR_SAND.g, COLOR_SAND.b)) {
                    result[index] = TileType::Sand;
                } else if (c.equals_rgb(COLOR_WATER.r, COLOR_WATER.g, COLOR_WATER.b)) {
                    result[index] = TileType::Water;
                } else if (c.equals_rgb(COLOR_SLOPE.r, COLOR_SLOPE.g, COLOR_SLOPE.b)) {
                    // Match BaseBlockFactory slope orientation rules
                    if (is_edge(colors, x + 1, y) && is_edge(colors, x, y + 1)) {
                        result[index] = TileType::SlopeTopLeft;
                    } else if (is_edge(colors, x - 1, y) && is_edge(colors, x, y + 1)) {
                        result[index] = TileType::SlopeTopRight;
                    } else if (is_edge(colors, x + 1, y) && is_edge(colors, x, y - 1)) {
                        result[index] = TileType::SlopeBottomLeft;
                    } else if (is_edge(colors, x - 1, y) && is_edge(colors, x, y - 1)) {
                        result[index] = TileType::SlopeBottomRight;
                    } else {
                        result[index] = TileType::SlopeTopRight; // Default fallback
                    }
                } else {
                    result[index] = TileType::Empty;
                }
            }
        }
        return result;
    }
};

} // namespace JumpKing
