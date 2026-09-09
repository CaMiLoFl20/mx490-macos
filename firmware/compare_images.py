#!/usr/bin/env python3
"""Compare two decoded firmware images without changing either input."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


def digest(path: Path) -> dict[str, object]:
    data = path.read_bytes()
    return {
        "path": str(path),
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("reference", type=Path)
    ap.add_argument("candidate", type=Path)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    a = args.reference.read_bytes()
    b = args.candidate.read_bytes()
    limit = min(len(a), len(b))
    changed = sum(x != y for x, y in zip(a[:limit], b[:limit]))
    result = {
        "reference": digest(args.reference),
        "candidate": digest(args.candidate),
        "common_prefix_bytes": next((i for i, (x, y) in enumerate(zip(a, b)) if x != y), limit),
        "changed_bytes_in_common_range": changed,
        "length_delta": len(b) - len(a),
    }
    text = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    else:
        print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
