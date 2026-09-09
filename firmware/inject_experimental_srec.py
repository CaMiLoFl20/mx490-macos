#!/usr/bin/env python3
"""Insert a probe payload into a verified empty Canon S-record range.

This emits an experimental, non-flashable stream. It refuses overlaps, refuses
addresses outside the SF-declared window, preserves wrapper/start records, and
validates every standard S-record checksum after insertion.
"""
from __future__ import annotations
import argparse, hashlib, json
from pathlib import Path

def s3(address: int, data: bytes) -> str:
    count = 4 + len(data) + 1
    body = bytes([count]) + address.to_bytes(4, "big") + data
    checksum = (~sum(body)) & 0xFF
    return "S3" + (body + bytes([checksum])).hex().upper()

def parse_std(line: str):
    if not line.startswith("S") or len(line) < 4 or line[1] == "F": return None
    raw = bytes.fromhex(line[2:]); count = raw[0]
    if count != len(raw) - 1 or sum(raw) & 0xFF != 0xFF: raise ValueError("invalid S-record")
    widths = {"1": 2, "2": 3, "3": 4, "5": 2, "6": 3, "7": 4, "8": 3, "9": 2}
    typ = line[1]
    if typ not in widths: raise ValueError(f"unsupported S{typ}")
    aw = widths[typ]; addr = int.from_bytes(raw[1:1 + aw], "big"); data = raw[1 + aw:-1]
    return typ, addr, len(data)

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("stream", type=Path)
    ap.add_argument("payload", type=Path)
    ap.add_argument("--base", required=True, type=lambda x: int(x, 0))
    ap.add_argument("-o", "--output", type=Path, required=True)
    ap.add_argument("--manifest", type=Path)
    args = ap.parse_args()
    lines = [x.strip() for x in args.stream.read_text().splitlines() if x.strip()]
    payload = args.payload.read_bytes(); start, end = args.base, args.base + len(payload)
    occupied = []
    declared_start = declared_end = None
    for line in lines:
        if line.startswith("SF0C"):
            raw = bytes.fromhex(line[2:]); declared_start = int.from_bytes(raw[4:8], "big"); declared_end = int.from_bytes(raw[8:12], "big") + 1
        rec = parse_std(line)
        if rec and rec[0] in {"1", "2", "3"}: occupied.append((rec[1], rec[1] + rec[2]))
    if declared_start is None or not (declared_start <= start and end <= declared_end): raise ValueError("payload outside SF-declared address window")
    if any(start < z and a < end for a, z in occupied): raise ValueError("payload overlaps existing S-record data")
    records = [s3(start + off, payload[off:off + 32]) for off in range(0, len(payload), 32)]
    insert_at = next((i for i, line in enumerate(lines) if line.startswith(("S7", "S8", "S9"))), len(lines))
    out_lines = lines[:insert_at] + records + lines[insert_at:]
    # Re-validate all standard records, including generated records.
    for line in out_lines: parse_std(line)
    args.output.write_text("\n".join(out_lines) + "\n")
    manifest = {"source": str(args.stream), "payload": str(args.payload), "payload_base": hex(start), "payload_end_exclusive": hex(end), "payload_size": len(payload), "payload_sha256": hashlib.sha256(payload).hexdigest(), "output": str(args.output), "output_sha256": hashlib.sha256(args.output.read_bytes()).hexdigest(), "generated_s3_records": len(records), "flashable": False}
    if args.manifest: args.manifest.write_text(json.dumps(manifest, indent=2) + "\n")
    print(json.dumps(manifest, indent=2)); return 0
if __name__ == "__main__": raise SystemExit(main())
