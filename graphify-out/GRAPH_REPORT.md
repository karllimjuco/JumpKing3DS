# Graph Report - JumpKing3DS  (2026-11-22)

## Corpus Check
- 15 files · ~8,013 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 4 file(s) not represented in the graph (top: (none) 3, .t3s 1)

## Summary
- 267 nodes · 397 edges · 11 communities
- Extraction: 95% EXTRACTED · 5% INFERRED · 0% AMBIGUOUS · INFERRED: 20 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `19418a2c`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- LevelRenderer
- JumpPhysics
- JumpPhysics.hpp
- os
- AdvCollisionInfo
- LevelTextureParser
- InputBuffer
- Vec2
- JumpKing3DS
- PlayerAnimFrame
- BlockFlags

## God Nodes (most connected - your core abstractions)
1. `JumpPhysics` - 48 edges
2. `LevelRenderer` - 43 edges
3. `TileType` - 14 edges
4. `PlayerAnimFrame` - 14 edges
5. `LevelTextureParser` - 14 edges
6. `InputBuffer` - 13 edges
7. `AdvCollisionInfo` - 13 edges
8. `Vec2` - 12 edges
9. `InputState` - 9 edges
10. `ColorRGBA` - 9 edges

## Surprising Connections (you probably didn't know these)
- `main()` --calls--> `get_slope_normal()`  [INFERRED]
  test/test_slope.cpp → src/AdvCollisionInfo.hpp
- `JumpPhysics` --references--> `Vec2`  [EXTRACTED]
  src/JumpPhysics.hpp → src/VectorMath.hpp
- `AdvCollisionInfo` --references--> `Vec2`  [EXTRACTED]
  src/AdvCollisionInfo.hpp → src/VectorMath.hpp
- `get_slope_normal()` --references--> `Vec2`  [EXTRACTED]
  src/AdvCollisionInfo.hpp → src/VectorMath.hpp

## Import Cycles
- None detected.

## Communities (11 total, 0 thin omitted)

### Community 0 - "LevelRenderer"
Cohesion: 0.06
Nodes (25): LevelRenderer, BLOCK_SIZE, BOTTOM_SCREEN_H, BOTTOM_SCREEN_W, COLS, LevelRenderer::LevelRenderer(), m_anim_timer, m_bg_image (+17 more)

### Community 1 - "JumpPhysics"
Cohesion: 0.06
Nodes (32): JumpPhysics, block_flags, BOUNCE, can_jump, charge_timer, current_state, DELTA_TIME, facing_direction (+24 more)

### Community 2 - "JumpPhysics.hpp"
Cohesion: 0.09
Nodes (11): FrameCoord, center_x, center_y, h, w, x, y, ColorRGB (+3 more)

### Community 3 - "os"
Cohesion: 0.15
Nodes (13): convert_level(), is_edge(), is_solid(), match_color(), process_king_sprites(), main(), unpack_xnb_texture(), convert_level_png_to_screens() (+5 more)

### Community 4 - "AdvCollisionInfo"
Cohesion: 0.09
Nodes (21): AdvCollisionInfo, has_collision, ice, quark, sand, slope_normal, slope_type, snow (+13 more)

### Community 5 - "LevelTextureParser"
Cohesion: 0.08
Nodes (27): TileType, Empty, Ice, Sand, SlopeBottomLeft, SlopeBottomRight, SlopeTopLeft, SlopeTopRight (+19 more)

### Community 6 - "InputBuffer"
Cohesion: 0.15
Nodes (8): InputBuffer, m_count, m_head, m_items, InputState, jump, left, right

### Community 7 - "Vec2"
Cohesion: 0.25
Nodes (7): UnitVectorPair2D, i, j, Vec2, x, y, VectorMath

### Community 8 - "JumpKing3DS"
Cohesion: 0.14
Nodes (13): 1. Prerequisites, 2. Run the Extractor, Asset Extraction Pipeline, Building the 3DS Homebrew, Compilation, Features, JumpKing3DS, Legal & Asset Disclaimer (BYOG) (+5 more)

### Community 9 - "PlayerAnimFrame"
Cohesion: 0.14
Nodes (14): PlayerAnimFrame, FRAME_IDLE, FRAME_JUMP_BOUNCE, FRAME_JUMP_CHARGE, FRAME_JUMP_FALL, FRAME_JUMP_UP, FRAME_LOOK_UP, FRAME_SPLAT (+6 more)

### Community 11 - "BlockFlags"
Cohesion: 0.18
Nodes (10): BlockFlags, is_ice, is_sand, is_snow, is_water, State, Airborne, Charging (+2 more)

## Knowledge Gaps
- **130 isolated node(s):** `x`, `y`, `width`, `height`, `left` (+125 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 164 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `JumpPhysics` connect `JumpPhysics` to `LevelRenderer`, `JumpPhysics.hpp`, `InputBuffer`, `Vec2`, `BlockFlags`?**
  _High betweenness centrality (0.344) - this node is a cross-community bridge._
- **What connects `x`, `y`, `width` to the rest of the system?**
  _130 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `LevelRenderer` be split into smaller, more focused modules?**
  _Cohesion score 0.05858585858585859 - nodes in this community are weakly interconnected._
- **Why does `LevelRenderer` connect `LevelRenderer` to `JumpPhysics.hpp`, `AdvCollisionInfo`, `LevelTextureParser`?**
  _High betweenness centrality (0.243) - this node is a cross-community bridge._
- **Should `JumpPhysics` be split into smaller, more focused modules?**
  _Cohesion score 0.06156156156156156 - nodes in this community are weakly interconnected._
- **Why does `Vec2` connect `Vec2` to `JumpPhysics`, `AdvCollisionInfo`?**
  _High betweenness centrality (0.094) - this node is a cross-community bridge._
- **Should `JumpPhysics.hpp` be split into smaller, more focused modules?**
  _Cohesion score 0.09359605911330049 - nodes in this community are weakly interconnected._