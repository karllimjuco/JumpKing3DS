#!/usr/bin/env python3
"""
XNB Texture2D Extractor & Tilemap Pipeline for 3DS Homebrew Ports
Extracts uncompressed Microsoft XNA / MonoGame .xnb Texture2D files into PNGs
and generates room binary grids from level collision masks.
"""

import os
import sys
import struct
import argparse
from PIL import Image

COLS = 60
ROWS = 45

# Jump King Level Color Codes
COLOR_SOLID = (0, 0, 0)
COLOR_SLOPE = (255, 0, 0)
COLOR_ICE   = (0, 255, 255)
COLOR_SNOW  = (255, 255, 0)
COLOR_SAND  = (255, 106, 0)
COLOR_WATER = (0, 170, 170)

def match_color(c, target, tol=15):
    return (abs(c[0] - target[0]) <= tol and
            abs(c[1] - target[1]) <= tol and
            abs(c[2] - target[2]) <= tol)

def is_edge(img, x, y, width, height):
    if x < 0 or x >= width or y < 0 or y >= height:
        return False
    c = img.getpixel((x, y))[:3]
    return not (match_color(c, COLOR_SOLID) or
                match_color(c, COLOR_SLOPE) or
                match_color(c, COLOR_ICE) or
                match_color(c, COLOR_SNOW) or
                match_color(c, COLOR_SAND))

def unpack_xnb_texture(xnb_path, out_png_path):
    """Parses uncompressed XNA 4.0 Texture2D .xnb binary files into standard PNGs."""
    with open(xnb_path, 'rb') as f:
        magic = f.read(3)
        if magic != b'XNB':
            raise ValueError(f"Not a valid XNB file: {xnb_path}")
        platform = f.read(1)
        version = f.read(1)[0]
        flags = f.read(1)[0]
        file_size = struct.unpack('<I', f.read(4))[0]

        if flags & 0x80:
            raise NotImplementedError(f"LZX compressed XNB files are not yet supported: {xnb_path}")

        def read_7bit():
            res = 0
            shift = 0
            while True:
                b = f.read(1)[0]
                res |= (b & 0x7F) << shift
                if (b & 0x80) == 0:
                    break
                shift += 7
            return res

        reader_count = read_7bit()
        for _ in range(reader_count):
            name_len = read_7bit()
            f.read(name_len)
            f.read(4)

        read_7bit() # shared resource count
        read_7bit() # primary type index

        surface_format = struct.unpack('<I', f.read(4))[0] # 0 = Color (RGBA)
        width = struct.unpack('<I', f.read(4))[0]
        height = struct.unpack('<I', f.read(4))[0]
        mip_count = struct.unpack('<I', f.read(4))[0]
        data_size = struct.unpack('<I', f.read(4))[0]

        raw_data = f.read(data_size)

    img = Image.frombytes('RGBA', (width, height), raw_data)
    os.makedirs(os.path.dirname(out_png_path), exist_ok=True)
    img.save(out_png_path)
    print(f"[EXTRACT] {os.path.basename(xnb_path)} -> {out_png_path} ({width}x{height})")
    return img

def crop_king_frames(base_png_path, out_frames_dir, out_t3s_path):
    """Crops player animation frames based on Jump King's Regular.cs layout."""
    img = Image.open(base_png_path)
    os.makedirs(out_frames_dir, exist_ok=True)

    frames = [
        ("idle",          224, 0,   48, 48),
        ("walk_one",      272, 0,   48, 48),
        ("walk_smear",    320, 0,   48, 48),
        ("walk_two",      368, 0,   48, 48),
        ("jump_charge",   224, 48,  48, 48),
        ("jump_up",       272, 48,  48, 48),
        ("jump_fall",     320, 48,  48, 48),
        ("splat",         368, 48,  48, 48),
        ("jump_bounce",   224, 96,  48, 48),
        ("look_up",       272, 96,  48, 48),
        ("stretch_one",   320, 96,  48, 48),
        ("stretch_smear", 368, 96,  48, 48),
        ("stretch_two",   224, 144, 48, 48)
    ]

    t3s_lines = ["--atlas", "-f rgba8888", "-z auto", ""]

    for name, x, y, w, h in frames:
        crop = img.crop((x, y, x + w, y + h))
        frame_file = os.path.join(out_frames_dir, f"{name}.png")
        crop.save(frame_file)
        t3s_lines.append(f"king_frames/{name}.png")

    with open(out_t3s_path, "w") as f:
        f.write("\n".join(t3s_lines) + "\n")
    print(f"[CROP] Generated 13 player animation frames in {out_frames_dir}")
    print(f"[T3S] Generated citro2d descriptor at {out_t3s_path}")

def convert_level_png_to_screens(level_png_path, out_screens_dir):
    """Converts Jump King level.png collision mask into 3DS room binary grid files."""
    img = Image.open(level_png_path).convert("RGBA")
    width, height = img.size
    grid_x = width // COLS
    grid_y = height // ROWS

    os.makedirs(out_screens_dir, exist_ok=True)

    screen_idx = 0
    for sy in range(grid_y):
        for sx in range(grid_x):
            room_bytes = bytearray(COLS * ROWS)

            for r in range(ROWS):
                for c_col in range(COLS):
                    gx = sx * COLS + c_col
                    gy = sy * ROWS + r
                    c = img.getpixel((gx, gy))[:3]

                    tile = 0
                    if match_color(c, COLOR_SOLID):
                        tile = 1
                    elif match_color(c, COLOR_ICE):
                        tile = 2
                    elif match_color(c, COLOR_SNOW):
                        tile = 3
                    elif match_color(c, COLOR_SAND):
                        tile = 4
                    elif match_color(c, COLOR_WATER):
                        tile = 5
                    elif match_color(c, COLOR_SLOPE):
                        if is_edge(img, gx - 1, gy, width, height) and is_edge(img, gx, gy - 1, width, height):
                            tile = 6
                        elif is_edge(img, gx + 1, gy, width, height) and is_edge(img, gx, gy - 1, width, height):
                            tile = 7
                        elif is_edge(img, gx - 1, gy, width, height) and is_edge(img, gx, gy + 1, width, height):
                            tile = 8
                        elif is_edge(img, gx + 1, gy, width, height) and is_edge(img, gx, gy + 1, width, height):
                            tile = 9
                        else:
                            tile = 7

                    room_bytes[r * COLS + c_col] = tile

            out_file = os.path.join(out_screens_dir, f"screen_{screen_idx:02d}.bin")
            with open(out_file, "wb") as f:
                f.write(room_bytes)
            screen_idx += 1

    print(f"[LEVEL] Generated {screen_idx} binary screen files in {out_screens_dir}")

def main():
    parser = argparse.ArgumentParser(description="XNB Texture2D Extractor & Tilemap Pipeline for 3DS")
    parser.add_argument("--content-dir", "-i", type=str, help="Path to Jump King Content folder containing .xnb files")
    parser.add_argument("--output-dir", "-o", type=str, default=".", help="Root of JumpKing3DS project")
    parser.add_argument("--single-xnb", "-s", type=str, help="Extract a single .xnb file to PNG")
    parser.add_argument("--out-png", type=str, help="Target PNG path when using --single-xnb")
    args = parser.parse_args()

    if args.single_xnb:
        out = args.out_png or os.path.splitext(args.single_xnb)[0] + ".png"
        unpack_xnb_texture(args.single_xnb, out)
        return

    if not args.content_dir:
        print("Usage: python xnb_extractor.py --content-dir <path_to_JumpKing_Content>")
        sys.exit(1)

    king_dir = os.path.join(args.content_dir, "king")
    gfx_dir = os.path.join(args.output_dir, "gfx")
    out_king_dir = os.path.join(gfx_dir, "extracted_king")
    king_frames_dir = os.path.join(gfx_dir, "king_frames")
    t3s_path = os.path.join(gfx_dir, "playersprites.t3s")
    levels_dir = os.path.join(args.output_dir, "romfs", "levels")
    screens_dir = os.path.join(args.output_dir, "romfs", "screens")

    # 1. Extract king textures
    if os.path.exists(king_dir):
        for fname in os.listdir(king_dir):
            if fname.lower().endswith(".xnb"):
                src = os.path.join(king_dir, fname)
                dst = os.path.join(out_king_dir, os.path.splitext(fname)[0] + ".png")
                unpack_xnb_texture(src, dst)

    # 2. Crop frames from base.png
    base_png = os.path.join(out_king_dir, "base.png")
    if os.path.exists(base_png):
        crop_king_frames(base_png, king_frames_dir, t3s_path)

    # 3. Extract level and slopes
    level_xnb = os.path.join(args.content_dir, "level.xnb")
    slopes_xnb = os.path.join(args.content_dir, "slopes.xnb")

    if os.path.exists(level_xnb):
        level_png = os.path.join(levels_dir, "level.png")
        unpack_xnb_texture(level_xnb, level_png)
        convert_level_png_to_screens(level_png, screens_dir)

    if os.path.exists(slopes_xnb):
        slopes_png = os.path.join(levels_dir, "slopes.png")
        unpack_xnb_texture(slopes_xnb, slopes_png)

    print("\n[SUCCESS] All Jump King assets extracted and prepared for 3DS build.")

if __name__ == "__main__":
    main()
