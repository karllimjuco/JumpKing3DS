#include <3ds.h>
#include <citro2d.h>
#include <cstdio>
#include "JumpPhysics.hpp"
#include "LevelRenderer.hpp"

using namespace JumpKing;

int main() {
    // 1. Initialize 3DS hardware subsystems
    Result romfs_res = romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // 2. Setup Citro2D dual screen render targets
    C3D_RenderTarget* top_target    = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom_target = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // Verify romfs file accessibility
    bool romfs_ok = R_SUCCEEDED(romfs_res);
    FILE* test_f = std::fopen("romfs:/screens/screen_00.bin", "rb");
    bool screen_file_ok = (test_f != nullptr);
    if (test_f) {
        std::fclose(test_f);
    }

    JumpKing::JumpPhysics player;
    JumpKing::LevelRenderer level;

    // Load initial room (screen 0)
    int current_room = 0;
    level.load_room(current_room);

    int jump_count = 0;
    int fall_count = 0;

    JumpKing::JumpPhysics::State prev_state = player.current_state;
    s16 prev_circle_dx = 0;

    // 3. Fixed 60Hz citro2d frame loop
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_START) {
            break; // Exit to Homebrew Launcher
        }

        if (kDown & (KEY_X | KEY_Y)) {
            level.toggle_collision_overlay();
        }

        circlePosition circle;
        hidCircleRead(&circle);

        // Map D-Pad and Circle Pad
        bool held_left  = (kHeld & (KEY_DLEFT | KEY_CPAD_LEFT)) || (circle.dx < -40);
        bool held_right = (kHeld & (KEY_DRIGHT | KEY_CPAD_RIGHT)) || (circle.dx > 40);
        bool held_jump  = (kHeld & (KEY_A | KEY_B));

        bool down_left  = (kDown & (KEY_DLEFT | KEY_CPAD_LEFT)) || (circle.dx < -40 && prev_circle_dx >= -40);
        bool down_right = (kDown & (KEY_DRIGHT | KEY_CPAD_RIGHT)) || (circle.dx > 40 && prev_circle_dx <= 40);
        bool down_jump  = (kDown & (KEY_A | KEY_B));

        prev_circle_dx = circle.dx;

        JumpKing::InputState input{
            .left = held_left,
            .right = held_right,
            .jump = held_jump
        };

        JumpKing::InputState pressed{
            .left = down_left,
            .right = down_right,
            .jump = down_jump
        };

        // 4. Run exact 60Hz physics tick with collision query
        player.tick(input, pressed, [&](const JumpKing::Rect& box, JumpKing::AdvCollisionInfo& info) {
            return level.query_collision(box, info);
        });

        // Track stats
        if (player.current_state == JumpKing::JumpPhysics::State::Charging &&
            prev_state != JumpKing::JumpPhysics::State::Charging) {
            jump_count++;
        }
        if (player.current_state == JumpKing::JumpPhysics::State::Splat &&
            prev_state != JumpKing::JumpPhysics::State::Splat) {
            fall_count++;
        }
        prev_state = player.current_state;

        // Room transition logic (screen height is 360 px)
        if (player.position.y < 0.0f) {
            // Jumped up into next room
            if (current_room < 44) {
                current_room++;
                player.position.y += JumpKing::LevelRenderer::ROOM_HEIGHT;
                level.load_room(current_room);
            } else {
                player.position.y = 0.0f;
                player.velocity.y = 0.0f;
            }
        } else if (player.position.y >= JumpKing::LevelRenderer::ROOM_HEIGHT) {
            // Fell down into previous room
            if (current_room > 0) {
                current_room--;
                player.position.y -= JumpKing::LevelRenderer::ROOM_HEIGHT;
                level.load_room(current_room);
            } else {
                player.position.y = JumpKing::LevelRenderer::ROOM_HEIGHT - JumpPhysics::PLAYER_HEIGHT - 16.0f;
                player.velocity.y = 0.0f;
            }
        }

        // 5. Dual-screen render
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        // Top Screen (400x240): World view with camera tracking
        // Diagnostic clear: dark blue (indicates rendering loop is alive)
        C2D_TargetClear(top_target, C2D_Color32(20, 32, 64, 255));
        C2D_SceneBegin(top_target);
        level.render_top_screen(player);

        // If romfs failed to mount or file missing, draw error indicator on top screen
        if (!romfs_ok || !screen_file_ok) {
            // Draw red warning bar at the top
            C2D_DrawRectSolid(0, 0, 0, 400, 20, C2D_Color32(220, 40, 40, 255));
        }

        // Bottom Screen (320x240): HUD & charge meter
        // Diagnostic clear: dark charcoal
        C2D_TargetClear(bottom_target, C2D_Color32(28, 30, 36, 255));
        C2D_SceneBegin(bottom_target);
        level.render_bottom_hud(player, current_room, jump_count, fall_count);

        C3D_FrameEnd(0);
    }

    // 6. Deinitialize subsystems cleanly
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    romfsExit();
    return 0;
}
