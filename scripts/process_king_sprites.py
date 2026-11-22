import os
from PIL import Image

def process_king_sprites():
    base_png = r"C:\JumpKing3DS\gfx\extracted_king\base.png"
    out_dir = r"C:\JumpKing3DS\gfx\king_frames"
    os.makedirs(out_dir, exist_ok=True)

    img = Image.open(base_png)

    # Coordinates from decompiled Regular.cs:
    # Point p_tile = new Point(48, 48);
    # Point p_offset = new Point(224, 0);
    frames = [
        ("idle",          224, 0,   48, 48),
        ("walk_one",      272, 0,   48, 48),
        ("walk_smear",    320, 0,   48, 48),
        ("walk_two",      368, 0,   48, 48),
        ("jump_charge",   224, 48,  48, 48),
        ("jump_up",       272, 48,  48, 48),
        ("jump_fall",     320, 48,  48, 48),
        ("jump_bounce",   224, 96,  48, 48),
        ("splat",         368, 48,  48, 48),
        ("look_up",       272, 96,  48, 48),
        ("stretch_one",   320, 96,  48, 48),
        ("stretch_smear", 368, 96,  48, 48),
        ("stretch_two",   224, 144, 48, 48)
    ]

    t3s_lines = [
        "--atlas",
        "-f rgba8888",
        "-z auto",
        ""
    ]

    for name, x, y, w, h in frames:
        crop = img.crop((x, y, x + w, y + h))
        frame_path = os.path.join(out_dir, f"{name}.png")
        crop.save(frame_path)
        rel_path = f"king_frames/{name}.png"
        t3s_lines.append(rel_path)
        print(f"Cropped frame {name}: ({x}, {y}, {w}, {h}) -> {frame_path}")

    t3s_path = r"C:\JumpKing3DS\gfx\playersprites.t3s"
    with open(t3s_path, "w") as f:
        f.write("\n".join(t3s_lines) + "\n")
    print(f"\nWrote tex3ds descriptor: {t3s_path}")

if __name__ == "__main__":
    process_king_sprites()
