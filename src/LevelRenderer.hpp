#pragma once

#include "JumpPhysics.hpp"
#include <citro2d.h>
#include <vector>
#include <string>
#include <cstdint>

namespace JumpKing {

enum class TileType : uint8_t {
    Empty = 0,
    Solid = 1,
    Ice = 2,
    Snow = 3,
    Sand = 4,
    Water = 5,
    SlopeTopLeft = 6,
    SlopeTopRight = 7,
    SlopeBottomLeft = 8,
    SlopeBottomRight = 9
};

enum PlayerAnimFrame : size_t {
    FRAME_IDLE = 0,
    FRAME_WALK_ONE = 1,
    FRAME_WALK_SMEAR = 2,
    FRAME_WALK_TWO = 3,
    FRAME_JUMP_CHARGE = 4,
    FRAME_JUMP_UP = 5,
    FRAME_JUMP_FALL = 6,
    FRAME_JUMP_BOUNCE = 7,
    FRAME_SPLAT = 8,
    FRAME_LOOK_UP = 9,
    FRAME_STRETCH_ONE = 10,
    FRAME_STRETCH_SMEAR = 11,
    FRAME_STRETCH_TWO = 12
};

struct FrameCoord {
    float x;
    float y;
    float w;
    float h;
    float center_x;
    float center_y;
};

// Original decompiled coordinates from Regular.cs (p_offset = {224, 0}, p_tile = {48, 48})
inline constexpr FrameCoord PLAYER_FRAMES[] = {
    {224.0f, 0.0f,   48.0f, 48.0f, 24.0f, 48.0f}, // 0: idle
    {272.0f, 0.0f,   48.0f, 48.0f, 24.0f, 48.0f}, // 1: walk_one
    {320.0f, 0.0f,   48.0f, 48.0f, 24.0f, 48.0f}, // 2: walk_smear
    {368.0f, 0.0f,   48.0f, 48.0f, 24.0f, 48.0f}, // 3: walk_two
    {224.0f, 48.0f,  48.0f, 48.0f, 24.0f, 48.0f}, // 4: jump_charge
    {272.0f, 48.0f,  48.0f, 48.0f, 24.0f, 48.0f}, // 5: jump_up
    {320.0f, 48.0f,  48.0f, 48.0f, 24.0f, 48.0f}, // 6: jump_fall
    {224.0f, 96.0f,  48.0f, 48.0f, 24.0f, 48.0f}, // 7: jump_bounce
    {368.0f, 48.0f,  48.0f, 48.0f, 24.0f, 48.0f}, // 8: splat
    {272.0f, 96.0f,  48.0f, 48.0f, 24.0f, 48.0f}, // 9: look_up
    {320.0f, 96.0f,  48.0f, 48.0f, 24.0f, 48.0f}, // 10: stretch_one
    {368.0f, 96.0f,  48.0f, 48.0f, 24.0f, 48.0f}, // 11: stretch_smear
    {224.0f, 144.0f, 48.0f, 48.0f, 24.0f, 48.0f}  // 12: stretch_two
};

class LevelRenderer {
public:
    static constexpr int BLOCK_SIZE = 8;
    static constexpr int COLS = 60;
    static constexpr int ROWS = 45;
    static constexpr int ROOM_WIDTH = COLS * BLOCK_SIZE;   // 480 px
    static constexpr int ROOM_HEIGHT = ROWS * BLOCK_SIZE; // 360 px

    static constexpr int TOP_SCREEN_W = 400;
    static constexpr int TOP_SCREEN_H = 240;
    static constexpr int BOTTOM_SCREEN_W = 320;
    static constexpr int BOTTOM_SCREEN_H = 240;

    LevelRenderer();
    ~LevelRenderer();

    bool load_room(int room_index);
    bool load_room_from_mask(int room_index);
    void create_fallback_room();

    bool query_collision(const Rect& hitbox, AdvCollisionInfo& out_info) const;
    void render_top_screen(const JumpPhysics& player);
    void render_bottom_hud(const JumpPhysics& player, int room_index, int jump_count, int fall_count);

    float get_camera_x() const { return m_camera_x; }
    float get_camera_y() const { return m_camera_y; }

private:
    std::vector<TileType> m_tiles;
    float m_camera_x = 0.0f;
    float m_camera_y = 0.0f;
    float m_anim_timer = 0.0f;

    C2D_TextBuf m_text_buf = nullptr;
    C2D_Text m_text_hud{};
    C2D_SpriteSheet m_sprite_sheet = nullptr;
    C2D_Sprite m_player_sprite{};

    u32 get_tile_color(TileType tile) const;
    void draw_player_sprite(const JumpPhysics& player, float px, float py);
};

} // namespace JumpKing
