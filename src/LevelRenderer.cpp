#include "LevelRenderer.hpp"
#include "LevelTextureParser.hpp"
#include <cstdio>
#include <algorithm>

namespace JumpKing {

LevelRenderer::LevelRenderer() {
    m_text_buf = C2D_TextBufNew(4096);
    m_sprite_sheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
    m_tiles.resize(COLS * ROWS, TileType::Empty);

    if (!load_room(0) && !load_room_from_mask(0)) {
        create_fallback_room();
    }
}

LevelRenderer::~LevelRenderer() {
    if (m_sprite_sheet) {
        C2D_SpriteSheetFree(m_sprite_sheet);
    }
    if (m_text_buf) {
        C2D_TextBufDelete(m_text_buf);
    }
}

bool LevelRenderer::load_room(int room_index) {
    char path[64];
    std::snprintf(path, sizeof(path), "romfs:/screens/screen_%02d.bin", room_index);

    FILE* f = std::fopen(path, "rb");
    if (!f) {
        return false;
    }

    std::size_t read_bytes = std::fread(m_tiles.data(), sizeof(TileType), COLS * ROWS, f);
    std::fclose(f);
    return read_bytes == (COLS * ROWS);
}

bool LevelRenderer::load_room_from_mask(int room_index) {
    char path[64];
    std::snprintf(path, sizeof(path), "romfs:/screens/mask_%02d.raw", room_index);

    FILE* f = std::fopen(path, "rb");
    if (!f) {
        return false;
    }

    std::vector<ColorRGB> colors(COLS * ROWS);
    std::size_t read_colors = std::fread(colors.data(), sizeof(ColorRGB), COLS * ROWS, f);
    std::fclose(f);

    if (read_colors == (COLS * ROWS)) {
        m_tiles = LevelTextureParser::parse_color_mask(colors);
        return true;
    }
    return false;
}

void LevelRenderer::create_fallback_room() {
    std::fill(m_tiles.begin(), m_tiles.end(), TileType::Empty);

    // Floor (bottom 2 rows: rows 43, 44)
    for (int col = 0; col < COLS; ++col) {
        m_tiles[43 * COLS + col] = TileType::Solid;
        m_tiles[44 * COLS + col] = TileType::Solid;
    }

    // Left and Right border walls
    for (int row = 0; row < ROWS; ++row) {
        m_tiles[row * COLS + 0] = TileType::Solid;
        m_tiles[row * COLS + 1] = TileType::Solid;
        m_tiles[row * COLS + (COLS - 2)] = TileType::Solid;
        m_tiles[row * COLS + (COLS - 1)] = TileType::Solid;
    }

    // Platforms at jump tiers
    for (int col = 10; col < 26; ++col) {
        m_tiles[36 * COLS + col] = TileType::Solid;
    }
    for (int col = 34; col < 48; ++col) {
        m_tiles[30 * COLS + col] = TileType::Ice;
    }

    // Slopes
    m_tiles[36 * COLS + 26] = TileType::SlopeTopRight;
    m_tiles[30 * COLS + 33] = TileType::SlopeTopLeft;

    for (int col = 16; col < 32; ++col) {
        m_tiles[22 * COLS + col] = TileType::Solid;
    }
}

bool LevelRenderer::query_collision(const Rect& hitbox, AdvCollisionInfo& out_info) const {
    out_info = AdvCollisionInfo{};

    // During screen transitions, hitbox may be above or below current room
    if (hitbox.y + hitbox.height <= 0.0f || hitbox.y >= ROOM_HEIGHT) {
        return false;
    }

    int start_col = std::clamp(static_cast<int>(hitbox.x / BLOCK_SIZE), 0, COLS - 1);
    int end_col   = std::clamp(static_cast<int>((hitbox.x + hitbox.width - 0.01f) / BLOCK_SIZE), 0, COLS - 1);
    int start_row = std::clamp(static_cast<int>(hitbox.y / BLOCK_SIZE), 0, ROWS - 1);
    int end_row   = std::clamp(static_cast<int>((hitbox.y + hitbox.height - 0.01f) / BLOCK_SIZE), 0, ROWS - 1);

    bool collided = false;

    for (int row = start_row; row <= end_row; ++row) {
        for (int col = start_col; col <= end_col; ++col) {
            TileType tile = m_tiles[row * COLS + col];
            if (tile == TileType::Empty) continue;

            collided = true;
            out_info.has_collision = true;

            switch (tile) {
                case TileType::Solid:
                    break;
                case TileType::Ice:
                    out_info.ice = true;
                    break;
                case TileType::Snow:
                    out_info.snow = true;
                    break;
                case TileType::Sand:
                    out_info.sand = true;
                    break;
                case TileType::Water:
                    out_info.water = true;
                    break;
                case TileType::SlopeTopLeft:
                    out_info.slope_type = SlopeType::TopLeft;
                    out_info.slope_normal = get_slope_normal(SlopeType::TopLeft);
                    break;
                case TileType::SlopeTopRight:
                    out_info.slope_type = SlopeType::TopRight;
                    out_info.slope_normal = get_slope_normal(SlopeType::TopRight);
                    break;
                case TileType::SlopeBottomLeft:
                    out_info.slope_type = SlopeType::BottomLeft;
                    out_info.slope_normal = get_slope_normal(SlopeType::BottomLeft);
                    break;
                case TileType::SlopeBottomRight:
                    out_info.slope_type = SlopeType::BottomRight;
                    out_info.slope_normal = get_slope_normal(SlopeType::BottomRight);
                    break;
                default:
                    break;
            }
        }
    }
    return collided;
}

u32 LevelRenderer::get_tile_color(TileType tile) const {
    switch (tile) {
        case TileType::Solid:            return C2D_Color32(110, 110, 120, 255);
        case TileType::Ice:              return C2D_Color32(140, 210, 255, 255);
        case TileType::Snow:             return C2D_Color32(240, 245, 255, 255);
        case TileType::Sand:             return C2D_Color32(230, 200, 120, 255);
        case TileType::Water:            return C2D_Color32(40,  100, 220, 180);
        case TileType::SlopeTopLeft:
        case TileType::SlopeTopRight:
        case TileType::SlopeBottomLeft:
        case TileType::SlopeBottomRight: return C2D_Color32(150, 140, 130, 255);
        default:                         return 0;
    }
}

void LevelRenderer::draw_player_sprite(const JumpPhysics& player, float px, float py) {
    PlayerAnimFrame frame = FRAME_IDLE;

    switch (player.current_state) {
        case JumpPhysics::State::Charging:
            frame = FRAME_JUMP_CHARGE;
            break;
        case JumpPhysics::State::Airborne:
            if (player.velocity.y < 0.0f) {
                frame = FRAME_JUMP_UP;
            } else if (player.is_knocked) {
                frame = FRAME_JUMP_BOUNCE;
            } else {
                frame = FRAME_JUMP_FALL;
            }
            break;
        case JumpPhysics::State::Splat:
            frame = FRAME_SPLAT;
            break;
        case JumpPhysics::State::Grounded:
            if (player.velocity.x != 0.0f) {
                m_anim_timer += JumpPhysics::DELTA_TIME * 8.0f;
                int step = static_cast<int>(m_anim_timer) % 4;
                if (step == 0) frame = FRAME_WALK_ONE;
                else if (step == 1 || step == 3) frame = FRAME_WALK_SMEAR;
                else frame = FRAME_WALK_TWO;
            } else {
                m_anim_timer = 0.0f;
                frame = FRAME_IDLE;
            }
            break;
    }

    const FrameCoord& fc = PLAYER_FRAMES[frame];

    // Feet are anchored at (px + 9, py + 26) with original center pivot (24, 48)
    float origin_x = px + 9.0f;
    float origin_y = py + 26.0f;

    if (m_sprite_sheet && frame < C2D_SpriteSheetCount(m_sprite_sheet)) {
        C2D_SpriteFromSheet(&m_player_sprite, m_sprite_sheet, frame);
        C2D_SpriteSetCenter(&m_player_sprite, fc.center_x / fc.w, fc.center_y / fc.h);
        C2D_SpriteSetPos(&m_player_sprite, origin_x, origin_y);

        float scale_x = (player.facing_direction == -1) ? -1.0f : 1.0f;
        C2D_SpriteSetScale(&m_player_sprite, scale_x, 1.0f);
        C2D_DrawSprite(&m_player_sprite);
    } else {
        // Geometric King fallback
        u32 body_color = player.is_knocked ? C2D_Color32(230, 80, 80, 255) : C2D_Color32(210, 170, 70, 255);
        float cape_x = (player.facing_direction == 1) ? px : (px + 12);
        C2D_DrawRectSolid(cape_x, py + 8, 0, 6, 16, C2D_Color32(180, 30, 40, 255));
        C2D_DrawRectSolid(px + 3, py + 6, 0, 12, 18, body_color);
        C2D_DrawRectSolid(px + 4, py, 0, 10, 8, C2D_Color32(235, 205, 90, 255));
        C2D_DrawRectSolid(px + 8, py - 3, 0, 2, 4, C2D_Color32(255, 240, 120, 255));
    }

    // In-game jump charge bar
    if (player.current_state == JumpPhysics::State::Charging) {
        float intensity = std::min(1.0f, player.charge_timer / JumpPhysics::JUMP_TIME);
        float meter_w = 24.0f;
        float meter_h = 4.0f;
        float meter_x = px - 3.0f;
        float meter_y = py - 8.0f;

        C2D_DrawRectSolid(meter_x - 1, meter_y - 1, 0, meter_w + 2, meter_h + 2, C2D_Color32(0, 0, 0, 220));
        u32 fill_color = (intensity < 0.5f) ? C2D_Color32(50, 220, 50, 255)
                                            : ((intensity < 0.9f) ? C2D_Color32(240, 200, 40, 255)
                                                                  : C2D_Color32(240, 40, 40, 255));
        C2D_DrawRectSolid(meter_x, meter_y, 0, meter_w * intensity, meter_h, fill_color);
    }
}

void LevelRenderer::render_top_screen(const JumpPhysics& player) {
    m_camera_x = std::clamp(player.position.x - (TOP_SCREEN_W / 2.0f) + (JumpPhysics::PLAYER_WIDTH / 2.0f),
                            0.0f, static_cast<float>(ROOM_WIDTH - TOP_SCREEN_W));
    m_camera_y = std::clamp(player.position.y - (TOP_SCREEN_H / 2.0f) + (JumpPhysics::PLAYER_HEIGHT / 2.0f),
                            0.0f, static_cast<float>(ROOM_HEIGHT - TOP_SCREEN_H));

    C2D_DrawRectSolid(0, 0, 0, TOP_SCREEN_W, TOP_SCREEN_H, C2D_Color32(18, 16, 26, 255));

    int start_col = std::max(0, static_cast<int>(m_camera_x / BLOCK_SIZE));
    int end_col   = std::min(COLS - 1, static_cast<int>((m_camera_x + TOP_SCREEN_W) / BLOCK_SIZE) + 1);
    int start_row = std::max(0, static_cast<int>(m_camera_y / BLOCK_SIZE));
    int end_row   = std::min(ROWS - 1, static_cast<int>((m_camera_y + TOP_SCREEN_H) / BLOCK_SIZE) + 1);

    for (int row = start_row; row <= end_row; ++row) {
        for (int col = start_col; col <= end_col; ++col) {
            TileType tile = m_tiles[row * COLS + col];
            if (tile == TileType::Empty) continue;

            float x = col * BLOCK_SIZE - m_camera_x;
            float y = row * BLOCK_SIZE - m_camera_y;
            u32 color = get_tile_color(tile);

            if (tile == TileType::SlopeTopLeft) {
                C2D_DrawTriangle(x + BLOCK_SIZE, y, color,
                                 x + BLOCK_SIZE, y + BLOCK_SIZE, color,
                                 x, y + BLOCK_SIZE, color, 0);
            } else if (tile == TileType::SlopeTopRight) {
                C2D_DrawTriangle(x, y, color,
                                 x + BLOCK_SIZE, y + BLOCK_SIZE, color,
                                 x, y + BLOCK_SIZE, color, 0);
            } else {
                C2D_DrawRectSolid(x, y, 0, BLOCK_SIZE, BLOCK_SIZE, color);
            }
        }
    }

    float px = player.position.x - m_camera_x;
    float py = player.position.y - m_camera_y;
    draw_player_sprite(player, px, py);
}

void LevelRenderer::render_bottom_hud(const JumpPhysics& player, int room_index, int jump_count, int fall_count) {
    C2D_DrawRectSolid(0, 0, 0, BOTTOM_SCREEN_W, BOTTOM_SCREEN_H, C2D_Color32(16, 18, 24, 255));

    C2D_DrawRectSolid(4, 4, 0, BOTTOM_SCREEN_W - 8, 2, C2D_Color32(60, 68, 85, 255));
    C2D_DrawRectSolid(4, BOTTOM_SCREEN_H - 6, 0, BOTTOM_SCREEN_W - 8, 2, C2D_Color32(60, 68, 85, 255));
    C2D_DrawRectSolid(4, 4, 0, 2, BOTTOM_SCREEN_H - 8, C2D_Color32(60, 68, 85, 255));
    C2D_DrawRectSolid(BOTTOM_SCREEN_W - 6, 4, 0, 2, BOTTOM_SCREEN_H - 8, C2D_Color32(60, 68, 85, 255));

    C2D_TextBufClear(m_text_buf);

    char buf[256];
    const char* state_str = "GROUNDED";
    if (player.current_state == JumpPhysics::State::Charging) state_str = "CHARGING";
    else if (player.current_state == JumpPhysics::State::Airborne) state_str = "AIRBORNE";
    else if (player.current_state == JumpPhysics::State::Splat) state_str = "SPLAT";

    std::snprintf(buf, sizeof(buf),
                  "--- JUMP KING 3DS ---\n\n"
                  "ROOM: %02d\n"
                  "STATE: %s\n"
                  "JUMPS: %d\n"
                  "FALLS: %d\n\n"
                  "POS: (%.1f, %.1f)\n"
                  "VEL: (%.2f, %.2f)",
                  room_index, state_str, jump_count, fall_count,
                  player.position.x, player.position.y,
                  player.velocity.x, player.velocity.y);

    C2D_TextParse(&m_text_hud, m_text_buf, buf);
    C2D_DrawText(&m_text_hud, C2D_WithColor, 16.0f, 16.0f, 0.5f, 0.55f, 0.55f, C2D_Color32(230, 235, 245, 255));

    float bar_x = 16.0f;
    float bar_y = 190.0f;
    float bar_w = BOTTOM_SCREEN_W - 32.0f;
    float bar_h = 18.0f;

    C2D_DrawRectSolid(bar_x - 2, bar_y - 2, 0, bar_w + 4, bar_h + 4, C2D_Color32(40, 44, 55, 255));
    C2D_DrawRectSolid(bar_x, bar_y, 0, bar_w, bar_h, C2D_Color32(20, 22, 28, 255));

    float intensity = std::min(1.0f, player.charge_timer / JumpPhysics::JUMP_TIME);
    if (intensity > 0.0f) {
        u32 bar_color = (intensity < 0.5f) ? C2D_Color32(60, 220, 80, 255)
                                           : ((intensity < 0.9f) ? C2D_Color32(245, 210, 50, 255)
                                                                 : C2D_Color32(245, 60, 60, 255));
        C2D_DrawRectSolid(bar_x, bar_y, 0, bar_w * intensity, bar_h, bar_color);
    }
}

} // namespace JumpKing
