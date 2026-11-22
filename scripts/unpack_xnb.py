import os
import struct
import sys
from PIL import Image

def unpack_xnb_texture(xnb_path, out_png_path):
    with open(xnb_path, 'rb') as f:
        magic = f.read(3)
        if magic != b'XNB':
            raise ValueError(f"Invalid XNB magic: {magic}")
        platform = f.read(1)
        version = f.read(1)[0]
        flags = f.read(1)[0]
        file_size = struct.unpack('<I', f.read(4))[0]

        if flags & 0x80:
            raise NotImplementedError("Compressed XNB not supported directly")

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
        readers = []
        for _ in range(reader_count):
            name_len = read_7bit()
            name = f.read(name_len).decode('ascii', errors='ignore')
            reader_ver = struct.unpack('<I', f.read(4))[0]
            readers.append((name, reader_ver))

        shared_count = read_7bit()
        type_idx = read_7bit()

        surface_format = struct.unpack('<I', f.read(4))[0] # 0 = Color (RGBA)
        width = struct.unpack('<I', f.read(4))[0]
        height = struct.unpack('<I', f.read(4))[0]
        mip_count = struct.unpack('<I', f.read(4))[0]
        data_size = struct.unpack('<I', f.read(4))[0]

        raw_data = f.read(data_size)

    # SurfaceFormat 0 is Color (RGBA in XNA / DirectX)
    # XNA Color is R, G, B, A (32-bit)
    if surface_format == 0:
        img = Image.frombytes('RGBA', (width, height), raw_data)
    else:
        # Fallback raw RGBA
        img = Image.frombytes('RGBA', (width, height), raw_data[:width * height * 4])

    os.makedirs(os.path.dirname(out_png_path), exist_ok=True)
    img.save(out_png_path)
    print(f"Extracted: {os.path.basename(xnb_path)} -> {out_png_path} ({width}x{height}, {surface_format})")
    return width, height

def main():
    content_dir = r"C:\Users\Karl Lopez\Downloads\Jump.King.v2025.05.30\Jump.King.v2025.05.30\Jump.King.v2025.05.30\Content"
    king_dir = os.path.join(content_dir, "king")
    out_king_dir = r"C:\JumpKing3DS\gfx\extracted_king"
    out_levels_dir = r"C:\JumpKing3DS\romfs\levels"

    os.makedirs(out_king_dir, exist_ok=True)
    os.makedirs(out_levels_dir, exist_ok=True)

    # 1. Extract all .xnb sprite sheets inside Content\king
    print("--- Extracting King Sprites ---")
    for fname in os.listdir(king_dir):
        if fname.lower().endswith(".xnb"):
            src = os.path.join(king_dir, fname)
            dst = os.path.join(out_king_dir, os.path.splitext(fname)[0] + ".png")
            unpack_xnb_texture(src, dst)

    # 2. Extract Content\level.xnb and Content\slopes.xnb
    print("\n--- Extracting Level & Slopes ---")
    level_src = os.path.join(content_dir, "level.xnb")
    slopes_src = os.path.join(content_dir, "slopes.xnb")

    unpack_xnb_texture(level_src, os.path.join(out_levels_dir, "level.png"))
    unpack_xnb_texture(slopes_src, os.path.join(out_levels_dir, "slopes.png"))

if __name__ == "__main__":
    main()
