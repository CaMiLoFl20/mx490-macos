#!/usr/bin/env python3
"""Extract length-prefixed zlib blocks from a decoded Canon S-record image.

The MX490 image stores several large runtime components as:

    uint32_le compressed_length
    78 9c ... zlib stream

This tool only extracts and hashes those blocks. It does not modify an image
or produce a flashable update.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import struct
import zlib
from pathlib import Path


def extract(image: bytes, out_dir: Path, min_size: int = 64 * 1024) -> list[dict]:
    out_dir.mkdir(parents=True, exist_ok=True)
    rows: list[dict] = []
    pos = 0
    while True:
        pos = image.find(b"\x78\x9c", pos)
        if pos < 4:
            if pos < 0:
                break
            pos += 2
            continue
        compressed_length = struct.unpack_from("<I", image, pos - 4)[0]
        if compressed_length <= 2 or pos + compressed_length > len(image):
            pos += 2
            continue
        stream = image[pos : pos + compressed_length]
        obj = zlib.decompressobj()
        try:
            decoded = obj.decompress(stream)
        except zlib.error:
            pos += 2
            continue
        consumed = compressed_length - len(obj.unused_data)
        if not obj.eof or consumed != compressed_length or len(decoded) < min_size:
            pos += 2
            continue
        name = f"block_{pos:08x}.bin"
        (out_dir / name).write_bytes(decoded)
        rows.append(
            {
                "offset": pos,
                "compressed_length": compressed_length,
                "decoded_length": len(decoded),
                "decoded_sha256": hashlib.sha256(decoded).hexdigest(),
                "file": name,
            }
        )
        pos += compressed_length
    return rows


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("image", type=Path)
    ap.add_argument("out_dir", type=Path)
    ap.add_argument("--json", type=Path, help="write an inventory JSON file")
    args = ap.parse_args()
    rows = extract(args.image.read_bytes(), args.out_dir)
    payload = {"image": str(args.image), "blocks": rows}
    if args.json:
        args.json.write_text(json.dumps(payload, indent=2) + "\n")
    print(json.dumps(payload, indent=2))


if __name__ == "__main__":
    main()
