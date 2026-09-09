#!/usr/bin/env python3
"""Inventory scanner/network service strings in a decoded MX490 image."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


PATTERNS = {
    "scanner": re.compile(r"scan|scanner|pagescan|scanparam", re.I),
    "wsd": re.compile(r"wsd|web service", re.I),
    "bonjour": re.compile(r"bonjour|zeroconf|mdns", re.I),
    "http": re.compile(r"http|cgi|ks_http", re.I),
    "bjnp": re.compile(r"bjnp|ijnp", re.I),
    "update": re.compile(r"firm.?update|update", re.I),
}


def strings(data: bytes, minimum: int = 6):
    for match in re.finditer(rb"[ -~]{%d,}" % minimum, data):
        yield match.start(), match.group().decode("ascii", "replace")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("image", type=Path)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    rows = []
    for offset, value in strings(args.image.read_bytes()):
        groups = [name for name, pattern in PATTERNS.items() if pattern.search(value)]
        if groups:
            rows.append({"offset": offset, "groups": groups, "value": value[:240]})
    result = {"image": str(args.image), "matches": rows}
    text = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    print(text, end="")


if __name__ == "__main__":
    main()
