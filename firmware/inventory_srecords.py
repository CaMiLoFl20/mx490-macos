#!/usr/bin/env python3
"""Validate and summarize a Canon S-record firmware stream.

The Canon updater's decrypted payload is an ASCII S-record stream.  This tool
does not decode or rewrite it; it checks record checksums and reports the
address ranges and start records needed to relate file offsets to load
addresses during later reverse engineering.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path


def parse_record(line: bytes) -> dict[str, int | str]:
    text = line.strip().decode("ascii")
    if not text.startswith("S") or len(text) < 4:
        raise ValueError("not an S-record")
    record_type = text[1]
    # Canon wraps the normal Motorola records with a small proprietary SF
    # header/trailer.  Preserve these records for auditing but do not apply
    # Motorola checksum rules to them.
    if record_type == "F":
        return {"type": "SF", "address": 0, "data_length": 0,
                "line_length": len(line.strip())}
    raw = bytes.fromhex(text[2:])
    count = raw[0]
    if count != len(raw) - 1:
        raise ValueError(f"byte count mismatch: {text[:12]}")
    if (sum(raw) & 0xFF) != 0xFF:
        raise ValueError(f"checksum mismatch: {text[:12]}")
    address_width = {"0": 2, "1": 2, "2": 3, "3": 4,
                     "5": 2, "6": 3, "7": 4, "8": 3, "9": 2}.get(record_type)
    if address_width is None:
        raise ValueError(f"unsupported record type S{record_type}")
    address = int.from_bytes(raw[1:1 + address_width], "big")
    payload_end = 1 + address_width
    return {
        "type": f"S{record_type}",
        "address": address,
        "data_length": max(0, count - address_width - 1),
        "line_length": len(line.strip()),
    }


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("stream", type=Path, help="decrypted ASCII S-record stream")
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    raw = args.stream.read_bytes()
    records = []
    errors = []
    for number, line in enumerate(raw.splitlines(), 1):
        if not line.strip():
            continue
        try:
            record = parse_record(line)
            record["line"] = number
            records.append(record)
        except ValueError as exc:
            errors.append({"line": number, "error": str(exc)})

    data_records = [r for r in records if r["type"] in {"S1", "S2", "S3"}]
    wrapper_records = [r for r in records if r["type"] == "SF"]
    starts = [r for r in records if r["type"] in {"S7", "S8", "S9"}]
    ranges = []
    if data_records:
        minimum = min(int(r["address"]) for r in data_records)
        maximum = max(int(r["address"]) + int(r["data_length"])
                      for r in data_records)
        ranges.append({
            "min_address": minimum,
            "min_address_hex": hex(minimum),
            "max_exclusive": maximum,
            "max_exclusive_hex": hex(maximum),
            "record_count": len(data_records),
        })
    for record in starts:
        record["address_hex"] = hex(int(record["address"]))
    result = {
        "stream": str(args.stream),
        "size": len(raw),
        "sha256": hashlib.sha256(raw).hexdigest(),
        "record_count": len(records),
        "data_record_count": len(data_records),
        "wrapper_record_count": len(wrapper_records),
        "valid": not errors,
        "errors": errors,
        "address_ranges": ranges,
        "start_records": starts,
    }
    text = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    print(text, end="")
    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())
