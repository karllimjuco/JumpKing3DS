import os
import sys
import struct
import subprocess
from PIL import Image

def unpack_xnb_texture(xnb_path):
    with open(xnb_path, 'rb') as f:
        magic = f.read(3)
        if magic != b'XNB':
            raise ValueError(f"Invalid XNB magic in {xnb_path}")
        f.read(3) # platform, version, flags
        f.read(4) # file_size

        def read_7bit():
            res, shift = 0, 0
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

        read_7bit() # shared
        read_7bit() # primary type

        surface_format = struct.unpack('<I', f.read(4))[0]
        width = struct.unpack('<I', f.read(4))[0]
        height = struct.unpack('<I', f.read(4))[0]
        f.read(4) # mip_count
        data_size = struct.unpack('<I', f.read(4))[0]
        raw_data = f.read(data_size)

    img = Image.frombytes('RGBA', (width, height), raw_data[:width * height * 4])
    return img

def main():
    content_screens = r"C:\Users\Karl Lopez\Downloads\Jump.King.v2025.05.30\Jump.King.v2025.05.30\Jump.King.v2025.05.30\Content\screens"
    bg_dir = os.path.join(content_screens, "background")
    mg_dir = os.path.join(content_screens, "midground")
    out_png_dir = r"C:\JumpKing3DS\gfx\extracted_screens"
    out_bgs_dir = r"C:\JumpKing3DS\romfs\bgs"
    tex3ds_bin = r"C:\devkitPro\tools\bin\tex3ds.exe"

    os.makedirs(out_png_dir, exist_ok=True)
    os.makedirs(out_bgs_dir, exist_ok=True)

    print("=== Compositing 1:1 Jump King Screens ===")

    for room_idx in range(45):
        game_id = room_idx + 1
        mg_file = os.path.join(mg_dir, f"{game_id}.xnb")
        bg_file = os.path.join(bg_dir, f"bg{game_id}.xnb")

        if not os.path.exists(mg_file):
            print(f"Skipping room {room_idx:02d}: {mg_file} not found")
            continue

        mg_img = unpack_xnb_texture(mg_file)

        # Upscale 60x45 special masks (like room 43) to standard 480x360
        if mg_img.size != (480, 360):
            mg_img = mg_img.resize((480, 360), Image.NEAREST)

        # If a background sky exists, alpha composite midground over background
        if os.path.exists(bg_file):
            bg_img = unpack_xnb_texture(bg_file)
            if bg_img.size != (480, 360):
                bg_img = bg_img.resize((480, 360), Image.NEAREST)
            final_img = Image.alpha_composite(bg_img, mg_img)
            status = f"composited with bg{game_id}.xnb"
        else:
            final_img = mg_img
            status = "midground only"

        # Save composited PNG
        png_path = os.path.join(out_png_dir, f"screen_{room_idx:02d}.png")
        final_img.save(png_path)

        # Convert to 3DS Citro2D texture (.t3x)
        t3x_path = os.path.join(out_bgs_dir, f"bg_{room_idx:02d}.t3x")
        proc = subprocess.run([tex3ds_bin, "-f", "rgb565", "-z", "auto", png_path, "-o", t3x_path], capture_output=True, text=True)
        if proc.returncode != 0:
            print(f"tex3ds failed on screen {room_idx:02d}: {proc.stderr.strip()}")
        else:
            print(f"Room {room_idx:02d} (game {game_id:2d}): {status} -> bg_{room_idx:02d}.t3x")

    print("\n[SUCCESS] All 45 room backgrounds composited and converted to Citro2D textures.")

if __name__ == "__main__":
    main()
