// C:\JumpKing3DS\test\test_parser.cpp
#include "../src/LevelTextureParser.hpp"
#include <cassert>
#include <iostream>

int main() {
    using namespace JumpKing;

    std::vector<ColorRGB> test_mask(LevelTextureParser::TOTAL_TILES, {255, 255, 255}); // White = empty

    // Place a solid block at (10, 10)
    test_mask[10 * LevelRenderer::COLS + 10] = LevelTextureParser::COLOR_SOLID;

    // Place an ice block at (15, 10)
    test_mask[10 * LevelRenderer::COLS + 15] = LevelTextureParser::COLOR_ICE;

    // Place a red slope block at (20, 10) with empty right (+1) and bottom (+1) neighbors
    test_mask[10 * LevelRenderer::COLS + 20] = LevelTextureParser::COLOR_SLOPE;

    std::vector<TileType> tiles = LevelTextureParser::parse_color_mask(test_mask);

    assert(tiles[10 * LevelRenderer::COLS + 10] == TileType::Solid);
    assert(tiles[10 * LevelRenderer::COLS + 15] == TileType::Ice);
    // Right and Bottom are empty -> TopLeft slope
    assert(tiles[10 * LevelRenderer::COLS + 20] == TileType::SlopeTopLeft);

    std::cout << "LevelTextureParser tests passed successfully.\n";
    return 0;
}
