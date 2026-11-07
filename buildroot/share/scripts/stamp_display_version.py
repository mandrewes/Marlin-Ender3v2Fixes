#!/usr/bin/env python3
"""
stamp_display_version.py
Stamps the firmware version string onto the DWIN boot image (0_start.jpg)
before copying to SD card, so you can visually confirm which display firmware
is loaded.

Usage: python3 stamp_display_version.py <version> <input_jpg> <output_jpg>
"""

import sys
from PIL import Image, ImageDraw, ImageFont

def stamp_version(version, input_path, output_path):
    img = Image.open(input_path).convert("RGB")
    draw = ImageDraw.Draw(img)

    # Try to use a monospace font, fall back to default
    font = None
    font_size = 18
    for font_path in [
        "/System/Library/Fonts/Supplemental/Courier New.ttf",
        "/System/Library/Fonts/Monaco.ttf",
        "/Library/Fonts/Arial.ttf",
    ]:
        try:
            font = ImageFont.truetype(font_path, font_size)
            break
        except (IOError, OSError):
            continue
    if font is None:
        font = ImageFont.load_default()

    text = f"FW: {version}"

    # Measure text size
    bbox = draw.textbbox((0, 0), text, font=font)
    text_w = bbox[2] - bbox[0]
    text_h = bbox[3] - bbox[1]

    margin = 8
    x = img.width - text_w - margin
    y = img.height - text_h - margin

    # Drop shadow for readability
    draw.text((x + 1, y + 1), text, font=font, fill=(0, 0, 0))
    # Cyan text to match Creality branding
    draw.text((x, y), text, font=font, fill=(0, 200, 220))

    img.save(output_path, "JPEG", quality=92)
    print(f"Stamped '{text}' onto {output_path}")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <version> <input_jpg> <output_jpg}")
        sys.exit(1)
    stamp_version(sys.argv[1], sys.argv[2], sys.argv[3])
