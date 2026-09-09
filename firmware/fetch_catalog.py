#!/usr/bin/env python3
"""Record Canon's public catalog metadata used by the MX490 update flow."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from urllib.request import Request, urlopen


URLS = {
    "version": "http://dtv-p.c-ij.com/sdata/struct01/version.bin",
    "trust_store": "http://dtv-p.c-ij.com/sdata/struct01/sdata.bin",
}


def fetch(url: str) -> dict[str, object]:
    request = Request(url, headers={"User-Agent": "IP Client/1.0.0.0"})
    with urlopen(request, timeout=30) as response:
        data = response.read()
        return {
            "url": response.url,
            "content_type": response.headers.get("Content-Type", ""),
            "last_modified": response.headers.get("Last-Modified", ""),
            "etag": response.headers.get("ETag", ""),
            "size": len(data),
            "sha256": hashlib.sha256(data).hexdigest(),
            "first_32_bytes_hex": data[:32].hex(),
        }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    result = {name: fetch(url) for name, url in URLS.items()}
    text = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
