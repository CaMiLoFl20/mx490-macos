#!/usr/bin/env python3
"""Find boot/decompressor address constants in a decoded image."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


DEFAULTS = [0x1DF9DE00, 0x108A780, 0xE8009B20, 0xE8006000, 0x04000000]


def hits(data: bytes, value: int, width: int, endian: str) -> list[int]:
    mask = (1 << (width * 8)) - 1
    needle = (value & mask).to_bytes(width, endian)
    result = []
    pos = 0
    while True:
        pos = data.find(needle, pos)
        if pos < 0:
            return result
        result.append(pos)
        pos += 1


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("image", type=Path)
    ap.add_argument("values", nargs="*", type=lambda x: int(x, 0), default=DEFAULTS)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    data = args.image.read_bytes()
    rows = []
    for value in args.values:
        for width in (3, 4):
            for endian in ("little", "big"):
                found = hits(data, value, width, endian)
                if found:
                    rows.append({"value": hex(value), "width": width,
                                 "endian": endian, "offsets": found})
    text = json.dumps({"image": str(args.image), "matches": rows}, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    print(text, end="")


if __name__ == "__main__":
    main()
