"""Generate transparent PNG and multi-resolution ICO assets for RemoteC.

The supplied master is composited on a nearly black matte.  This script removes
only that matte, preserving every bright glass/highlight pixel of the logo.
"""

from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[2]
BRANDING = ROOT / "assets" / "ui" / "branding"
SOURCE = BRANDING / "remotec-logo-source.png"
PNG_OUTPUT = BRANDING / "remotec-logo.png"
ICO_OUTPUT = BRANDING / "remotec-app.ico"
LEGACY_BRANDING = ROOT / "assets" / "branding"
WINDOWS_PNG_OUTPUT = LEGACY_BRANDING / "RemoteC-icon.png"
WINDOWS_ICO_OUTPUT = LEGACY_BRANDING / "RemoteC.ico"


def extract_black_matte(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    pixels = rgba.load()
    for y in range(rgba.height):
        for x in range(rgba.width):
            red, green, blue, _ = pixels[x, y]
            peak = max(red, green, blue)
            # The source background is 0-2 RGB. A short feather retains the
            # original antialiased cyan glow without leaving a black square.
            if peak <= 2:
                alpha = 0
            elif peak < 18:
                alpha = round((peak - 2) * 255 / 16)
            else:
                alpha = 255
            pixels[x, y] = (red, green, blue, alpha)
    return rgba


def main() -> None:
    BRANDING.mkdir(parents=True, exist_ok=True)
    with Image.open(SOURCE) as master:
        transparent = extract_black_matte(master)
    transparent.save(PNG_OUTPUT, optimize=True)
    LEGACY_BRANDING.mkdir(parents=True, exist_ok=True)
    transparent.save(WINDOWS_PNG_OUTPUT, optimize=True)
    icon_sizes = [(16, 16), (24, 24), (32, 32), (48, 48),
                  (64, 64), (128, 128), (256, 256)]
    for destination in (ICO_OUTPUT, WINDOWS_ICO_OUTPUT):
        transparent.save(destination, format="ICO", sizes=icon_sizes)
    print(f"PNG={PNG_OUTPUT}")
    print(f"ICO={ICO_OUTPUT}")
    print(f"WINDOWS_PNG={WINDOWS_PNG_OUTPUT}")
    print(f"WINDOWS_ICO={WINDOWS_ICO_OUTPUT}")


if __name__ == "__main__":
    main()
