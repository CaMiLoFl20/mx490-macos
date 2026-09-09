#!/usr/bin/env python3
"""Inventory Canon's opaque device-update metadata without decrypting it."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path


def entropy(data: bytes) -> float:
    if not data:
        return 0.0
    counts = [0] * 256
    for value in data:
        counts[value] += 1
    return -sum((n / len(data)) * math.log2(n / len(data)) for n in counts if n)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("manifest", type=Path)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    data = args.manifest.read_bytes()
    report = {
        "file": str(args.manifest),
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "md5": hashlib.md5(data).hexdigest(),
        "entropy_bits_per_byte": round(entropy(data), 5),
        "aes_block_aligned_payload": len(data) % 16 == 0,
        "ascii_strings": [
            part.decode("ascii", "replace")
            for part in data.split(b"\x00")
            if len(part) >= 4 and all(32 <= c < 127 for c in part)
        ],
        "first_32_bytes_hex": data[:32].hex(),
    }
    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    print(text, end="")


if __name__ == "__main__":
    main()
