#!/usr/bin/env python3
"""Generate the 128x24 QNEMU wordmark used by the CGB boot ROM.

The output is 2bpp planar tile data (48 8x8 tiles, 16 bytes each), in the
same left-to-right, top-to-bottom order the boot ROM's tilemap uses.

This script has no third-party dependencies. It is only needed when
regenerating the logo; the committed qnemu_logo.2bpp/qnemu_logo.pb12 files
are used by the normal build.
"""

from __future__ import annotations

from pathlib import Path

GLYPHS = {
    "Q": ["01110", "10001", "10001", "10001", "10101", "10010", "01101"],
    "N": ["10001", "11001", "10101", "10011", "10001", "10001", "10001"],
    "E": ["11111", "10000", "10000", "11110", "10000", "10000", "11111"],
    "M": ["10001", "11011", "10101", "10101", "10001", "10001", "10001"],
    "U": ["10001", "10001", "10001", "10001", "10001", "10001", "01110"],
}

WIDTH = 128
HEIGHT = 24
SCALE = 3
SPACING = 4
TEXT = "QNEMU"


def render() -> list[list[int]]:
    image = [[0] * WIDTH for _ in range(HEIGHT)]
    text_width = len(TEXT) * 5 * SCALE + (len(TEXT) - 1) * SPACING
    x0 = (WIDTH - text_width) // 2
    y0 = (HEIGHT - 7 * SCALE) // 2

    def put(x: int, y: int, color: int) -> None:
        if 0 <= x < WIDTH and 0 <= y < HEIGHT:
            image[y][x] = color

    # A 2px drop shadow in colour 1, then the glyphs in colour 2.
    for i, ch in enumerate(TEXT):
        glyph_x = x0 + i * (5 * SCALE + SPACING)
        for gy, row in enumerate(GLYPHS[ch]):
            for px, bit in enumerate(row):
                if bit != "1":
                    continue
                for dy in range(SCALE):
                    for dx in range(SCALE):
                        put(glyph_x + px * SCALE + dx + 2,
                            y0 + gy * SCALE + dy + 2, 1)

    for i, ch in enumerate(TEXT):
        glyph_x = x0 + i * (5 * SCALE + SPACING)
        for gy, row in enumerate(GLYPHS[ch]):
            for px, bit in enumerate(row):
                if bit != "1":
                    continue
                for dy in range(SCALE):
                    for dx in range(SCALE):
                        put(glyph_x + px * SCALE + dx,
                            y0 + gy * SCALE + dy, 2)

    return image


def to_2bpp(image: list[list[int]]) -> bytes:
    out = bytearray()
    for tile_y in range(HEIGHT // 8):
        for tile_x in range(WIDTH // 8):
            for row in range(8):
                low = 0
                high = 0
                for col in range(8):
                    value = image[tile_y * 8 + row][tile_x * 8 + col]
                    low = (low << 1) | (value & 1)
                    high = (high << 1) | ((value >> 1) & 1)
                out.append(low)
                out.append(high)
    assert len(out) == 768
    return bytes(out)


def main() -> None:
    out_path = Path(__file__).with_name("qnemu_logo.2bpp")
    out_path.write_bytes(to_2bpp(render()))
    print(f"wrote {out_path} ({out_path.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
