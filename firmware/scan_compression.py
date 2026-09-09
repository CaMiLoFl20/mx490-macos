#!/usr/bin/env python3
"""Inventory standard zlib streams and the known related-model Canon LZ signature."""
from __future__ import annotations
import argparse, hashlib, json, re, zlib
from pathlib import Path

CANON_LZ_SIGNATURE = bytes.fromhex("70 b5 05 4c 05 48 06 49 45 1a 0e 46 2a 46 31 46 20 46 ff")
ZLIB_HEADERS = {bytes((0x78, x)) for x in (0x01, 0x5E, 0x9C, 0xDA)}

def scan(path: Path) -> dict:
    data = path.read_bytes()
    streams = []
    seen = set()
    for off in range(len(data) - 1):
        if data[off:off + 2] not in ZLIB_HEADERS or off in seen:
            continue
        try:
            obj = zlib.decompressobj()
            decoded = obj.decompress(data[off:])
            if len(decoded) < 4096 or not obj.eof:
                continue
            consumed = len(data[off:]) - len(obj.unused_data)
            seen.add(off)
            streams.append({
                "offset": off,
                "compressed_length": consumed,
                "decoded_length": len(decoded),
                "decoded_sha256": hashlib.sha256(decoded).hexdigest(),
                "decoded_markers": {
                    marker.decode("ascii", "replace"): len(list(re.finditer(re.escape(marker), decoded)))
                    for marker in (b"eSCL", b"AirScan", b"WSD", b"SCAN", b"X-CISSE")
                    if marker in decoded
                },
            })
        except zlib.error:
            continue
    return {
        "image": str(path),
        "image_size": len(data),
        "known_canon_lz_signature": CANON_LZ_SIGNATURE.hex(),
        "canon_lz_signature_offsets": [m.start() for m in re.finditer(re.escape(CANON_LZ_SIGNATURE), data)],
        "zlib_streams": streams,
    }

def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("image", type=Path)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    report = scan(args.image)
    text = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    else:
        print(text, end="")

if __name__ == "__main__":
    main()
