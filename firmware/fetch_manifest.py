#!/usr/bin/env python3
"""Fetch the Canon MX490 update metadata without contacting the printer."""

from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
from urllib.request import Request, urlopen


URLS = (
    "https://gdlp01.c-wss.com/rmds/ij/ijd/ijdupdate/1787.bin",
    "http://gdlp01.c-wss.com/rmds/ij/ijd/ijdupdate/1787.bin",
)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("output", type=Path)
    args = ap.parse_args()
    response = None
    for url in URLS:
        try:
            req = Request(url, headers={"User-Agent": "IP Client/1.0.0.0"})
            response = urlopen(req, timeout=30)
            break
        except Exception:
            continue
    if response is None:
        raise RuntimeError("Canon manifest endpoint could not be fetched")
    with response:
        data = response.read()
    args.output.write_bytes(data)
    print(f"url={response.url}")
    print(f"last_modified={response.headers.get('Last-Modified', '')}")
    print(f"etag={response.headers.get('ETag', '')}")
    print(f"bytes={len(data)}")
    print(f"sha256={hashlib.sha256(data).hexdigest()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
