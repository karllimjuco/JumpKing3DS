# JumpKing3DS

A faithful C++17 homebrew port of **Jump King** for the Nintendo 3DS, built with **devkitARM**, **libctru**, and **citro2d**.

---

## Features

- **Pixel-Accurate 60 Hz Physics Engine**:
  - Exact Jump King vertical formulas: gravity accumulation (`153 / 595 ≈ 0.2571`), jump strength (`-34 × 153 / 595 ≈ -8.7428`), and 0.6s max charge timing.
  - Exact 4-frame directional jump input buffering.
  - 2D vector slope reflection using surface normal projections (`VectorMath::make_composite`).
  - Ice momentum, ice steering acceleration, and per-frame friction decay (`0.1f`).
  - Splat state locking upon landing at terminal velocity (`MAX_FALL = 10.0f`).
- **Dual-Screen Layout**:
  - **Top Screen (400×240)**: Active room viewport with smooth vertical camera tracking matching player height, 60×45 tilemap geometry, and animated player sprites.
  - **Bottom Screen (320×240)**: Diagnostic HUD displaying current room index, player physics state (`GROUNDED`, `CHARGING`, `AIRBORNE`, `SPLAT`), jump & fall counters, live coordinates, and an analog jump charge bar.
- **Controls**:
  - **Circle Pad / D-Pad**: Move left / right & buffer jump direction.
  - **A / B (Hold)**: Charge jump impulse.
  - **Start**: Exit to Homebrew Launcher.

---

## Legal & Asset Disclaimer (BYOG)

This repository contains **only original C++ source code, test suites, and extraction tools**. No copyrighted graphics, audio, or game data files from *Jump King* are distributed in this repository.

To play, you must provide your own legally purchased PC copy of **Jump King** (Steam / GOG). The included Python tool automatically extracts and formats the required assets.

---

## Asset Extraction Pipeline

### 1. Prerequisites
- Python 3.8+
- [Pillow](https://python-pillow.org/):
  ```bash
  pip install Pillow
  ```

### 2. Run the Extractor
Point the extraction script to your Jump King PC `Content` directory:
```bash
python tools/xnb_extractor.py --content-dir "path/to/JumpKing/Content"
```

The script will automatically:
1. Decode uncompressed XNA `.xnb` textures (`base.xnb`, `level.xnb`, `slopes.xnb`, and costume reskins) into PNG format.
2. Slice the 13 player animation frames (`idle`, `walk`, `jump_charge`, `jump_up`, `jump_fall`, `splat`, `bounce`, etc.) with correct bottom-center pivots into `gfx/king_frames/`.
3. Generate the citro2d texture atlas descriptor (`gfx/playersprites.t3s`).
4. Parse the 780×585 collision mask (`level.png`) into 169 room binary grids (`romfs/screens/screen_XX.bin`).

---

## Building the 3DS Homebrew

### Prerequisites
- [devkitPro](https://devkitpro.org/) with:
  - `devkitARM`
  - `libctru`
  - `citro2d`
  - `citro3d`
  - `tex3ds`
  - `3ds-tools`

### Compilation
Open your devkitPro / MSYS2 shell, navigate to this directory, and run:
```bash
make
```

This compiles:
- `JumpKing3DS.elf`
- `JumpKing3DS.smdh`
- `JumpKing3DS.3dsx` (with packaged `romfs` containing all level screens and sprite sheets)

### Running on 3DS
Copy `JumpKing3DS.3dsx` to `sdmc:/3ds/JumpKing3DS/` and launch via the **Homebrew Launcher**, or test with [Citra](https://citra-emu.org/).

---

## Native PC Test Suite (No devkitPro required)

The physics and collision routines have zero platform dependencies and can be compiled and validated natively on PC using any standard C++17 compiler (`g++` or `clang++`):

```bash
# Verify jump charge, gravity, and terminal velocities
g++ -std=c++17 test/test_jump.cpp -o test_jump.exe && ./test_jump.exe

# Verify horizontal wall collision, bouncing, and ice friction decay
g++ -std=c++17 test/test_horizontal.cpp -o test_horizontal.exe && ./test_horizontal.exe

# Verify vector slope normals and angle reflections
g++ -std=c++17 test/test_slope.cpp -o test_slope.exe && ./test_slope.exe

# Verify level color mask parser
g++ -std=c++17 test/test_parser.cpp -o test_parser.exe && ./test_parser.exe
```

---

## Project Structure

```
JumpKing3DS/
├── gfx/                  # Citro2d texture descriptors & cropped sprite assets
│   └── playersprites.t3s # Texture sheet definition for tex3ds
├── romfs/                # Packaged game data
│   └── screens/          # Binary room grids (screen_00.bin to screen_168.bin)
├── src/                  # Core C++17 source code
│   ├── AdvCollisionInfo.hpp
│   ├── JumpPhysics.hpp   # Pure 60Hz physics, jump charging & state machine
│   ├── LevelRenderer.hpp # Room tilemap querying & Citro2d dual-screen rendering
│   ├── LevelRenderer.cpp
│   ├── LevelTextureParser.hpp # Color-mask to tile index conversion
│   ├── VectorMath.hpp    # Slope vector projections & 2D surface reflections
│   └── main.cpp          # 3DS hardware initialization & Citro2d frame loop
├── test/                 # Native PC test suites (standalone g++ assertions)
├── tools/
│   └── xnb_extractor.py  # Standalone XNA .xnb extractor & level converter tool
├── Makefile              # devkitARM 3DS build script
└── README.md
```

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.  
*Jump King* is developed and published by **Nexile**. All rights to original assets belong to their respective owners.
