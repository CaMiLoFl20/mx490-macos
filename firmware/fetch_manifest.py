#!/usr/bin/env python3
"""Fetch the Canon MX490 update metadata without contacting the printer."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
from urllib.request import Request, urlopen


URL = "http://gdlp01.c-wss.com/rmds/ij/ijd/ijdupdate/1787.bin"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("output", type=Path)
    args = ap.parse_args()
    req = Request(URL, headers={"User-Agent": "IP Client/1.0.0.0"})
    with urlopen(req, timeout=30) as response:
        data = response.read()
    args.output.write_bytes(data)
    print(f"url={URL}")
    print(f"bytes={len(data)}")
    print(f"sha256={hashlib.sha256(data).hexdigest()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
