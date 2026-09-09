#!/usr/bin/env python3
"""Report ARM LDR-PC vector stubs in the MX490 boot prefix.

The MX490 image begins with ARM boot code and later compressed components.
This scanner only examines aligned ARM words in a bounded prefix; it does not
attempt to execute or patch firmware.  It records the literal-pool address and
the value loaded by each ``ldr pc, [pc, #imm]`` instruction, which helps
distinguish image-relative targets from low runtime/ROM targets.
"""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path


def scan(data: bytes, base: int, limit: int) -> list[dict[str, int | str]]:
    result = []
    end = min(limit, len(data) - 4)
    for offset in range(0, end, 4):
        word = struct.unpack_from("<I", data, offset)[0]
        # ARM single-data-transfer, L=1, Rn=PC, Rd=PC, immediate offset.
        if (word & 0x0F7FF000) != 0x051FF000:
            continue
        pc = base + offset + 8
        immediate = word & 0xFFF
        literal_address = pc + (immediate if word & 0x00800000 else -immediate)
        literal_offset = literal_address - base
        if not 0 <= literal_offset <= len(data) - 4:
            continue
        loaded = struct.unpack_from("<I", data, literal_offset)[0]
        result.append({
            "file_offset": offset,
            "runtime_address": hex(base + offset),
            "instruction": hex(word),
            "literal_offset": literal_offset,
            "literal_address": hex(literal_address),
            "loaded_value": loaded,
            "loaded_value_hex": hex(loaded),
            "target_class": "image-range" if base <= loaded < base + len(data) else "low-or-external",
        })
    return result


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("image", type=Path)
    ap.add_argument("--base", type=lambda value: int(value, 0), default=0xF0020000)
    ap.add_argument("--limit", type=lambda value: int(value, 0), default=0x5004)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    data = args.image.read_bytes()
    rows = scan(data, args.base, args.limit)
    result = {
        "image": str(args.image),
        "base": hex(args.base),
        "prefix_limit": args.limit,
        "vector_count": len(rows),
        "vectors": rows,
    }
    text = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
