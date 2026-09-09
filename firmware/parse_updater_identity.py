#!/usr/bin/env python3
"""Parse Canon updater identity records without contacting a printer."""
import argparse
import json
import re
from pathlib import Path

KEYS = ("MDL", "VER", "DES", "STA", "CMD")

def parse_record(data: bytes):
    text = data.decode("latin1", errors="replace")
    result = {}
    for key in KEYS:
        match = re.search(r"(?:^|;)" + key + r":([^;]*)", text, flags=re.IGNORECASE)
        if match:
            result[key] = match.group(1)
    return result

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("path", type=Path)
    args = ap.parse_args()
    print(json.dumps(parse_record(args.path.read_bytes()), indent=2, sort_keys=True))

if __name__ == "__main__":
    main()
