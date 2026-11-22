import os
from PIL import Image

COLS = 60
ROWS = 45

# Match LevelTextureParser Color definitions
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

def is_solid(img, x, y, width, height):
    if x < 0 or x >= width or y < 0 or y >= height:
        return True # Beyond screen edges is treated as edge/solid
    c = img.getpixel((x, y))
    if c[3] < 128:
        return False # Transparent air is NOT solid
    return (match_color(c[:3], COLOR_SOLID) or
            match_color(c[:3], COLOR_SLOPE) or
            match_color(c[:3], COLOR_ICE) or
            match_color(c[:3], COLOR_SNOW) or
            match_color(c[:3], COLOR_SAND))

def is_edge(img, x, y, width, height):
    if x < 0 or x >= width or y < 0 or y >= height:
        return False
    c = img.getpixel((x, y))
    if c[3] < 128:
        return True # Empty air counts as an edge
    return not (match_color(c[:3], COLOR_SOLID) or
                match_color(c[:3], COLOR_SLOPE) or
                match_color(c[:3], COLOR_ICE) or
                match_color(c[:3], COLOR_SNOW) or
                match_color(c[:3], COLOR_SAND))

def convert_level():
    level_png = r"C:\JumpKing3DS\romfs\levels\level.png"
    out_dir = r"C:\JumpKing3DS\romfs\screens"
    os.makedirs(out_dir, exist_ok=True)

    img = Image.open(level_png).convert("RGBA")
    width, height = img.size
    grid_x = width // COLS   # 13
    grid_y = height // ROWS  # 13

    print(f"Level dimensions: {width}x{height} -> {grid_x}x{grid_y} screens ({grid_x * grid_y} total)")

    total_screens = grid_x * grid_y
    for screen_idx in range(total_screens):
        # In Jump King LevelTexture.cs:
        # num = p_screen / square; num2 = p_screen - num * square;
        # coord = (num * TILE.X + x, num2 * TILE.Y + y)
        sx = screen_idx // grid_y
        sy = screen_idx % grid_y

        room_bytes = bytearray(COLS * ROWS)

        for r in range(ROWS):
            for col in range(COLS):
                gx = sx * COLS + col
                gy = sy * ROWS + r
                c = img.getpixel((gx, gy))

                tile = 0 # Empty

                # Transparent pixels (alpha < 128) are empty air
                if c[3] >= 128:
                    rgb = c[:3]
                    if match_color(rgb, COLOR_SOLID):
                        tile = 1 # Solid
                    elif match_color(rgb, COLOR_ICE):
                        tile = 2 # Ice
                    elif match_color(rgb, COLOR_SNOW):
                        tile = 3 # Snow
                    elif match_color(rgb, COLOR_SAND):
                        tile = 4 # Sand
                    elif match_color(rgb, COLOR_WATER):
                        tile = 5 # Water
                    elif match_color(rgb, COLOR_SLOPE):
                        # Match BaseBlockFactory slope orientation rules
                        if is_edge(img, gx + 1, gy, width, height) and is_edge(img, gx, gy + 1, width, height):
                            tile = 6 # SlopeTopLeft
                        elif is_edge(img, gx - 1, gy, width, height) and is_edge(img, gx, gy + 1, width, height):
                            tile = 7 # SlopeTopRight
                        elif is_edge(img, gx + 1, gy, width, height) and is_edge(img, gx, gy - 1, width, height):
                            tile = 8 # SlopeBottomLeft
                        elif is_edge(img, gx - 1, gy, width, height) and is_edge(img, gx, gy - 1, width, height):
                            tile = 9 # SlopeBottomRight
                        else:
                            tile = 7 # Fallback slope

                room_bytes[r * COLS + col] = tile

        out_file = os.path.join(out_dir, f"screen_{screen_idx:02d}.bin")
        with open(out_file, "wb") as f:
            f.write(room_bytes)

    print(f"Successfully generated {total_screens} binary screen files in {out_dir}")

if __name__ == "__main__":
    convert_level()
