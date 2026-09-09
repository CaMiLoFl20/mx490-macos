#!/usr/bin/env python3
"""Inventory a decoded Canon PIXMA firmware image without modifying it."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path


INTERESTING = re.compile(
    rb"(?i)(airscan|escl|uscan|bonjour|bjnp|ipp|http|wsd|scan|firm|update|dryos|itron)"
)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("image", type=Path, help="decoded firmware image")
    ap.add_argument("-o", "--output", type=Path, default=None)
    args = ap.parse_args()

    data = args.image.read_bytes()
    hits = []
    for match in re.finditer(rb"[ -~]{6,}", data):
        value = match.group().decode("ascii", "replace")
        if INTERESTING.search(match.group()):
            hits.append({"offset": match.start(), "value": value})

    result = {
        "image": str(args.image),
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "interesting_strings": hits,
        "airscan_strings_found": any(
            token in data.lower() for token in (b"airscan", b"escl", b"_uscan._tcp")
        ),
    }
    encoded = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(encoded)
    else:
        print(encoded, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
