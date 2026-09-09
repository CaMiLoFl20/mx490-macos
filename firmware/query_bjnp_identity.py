#!/usr/bin/env python3
"""Read the Canon BJNP IEEE-1284 identity from a printer.

This sends only the documented BJNP ``GetId`` query. It does not start a
print/scan job, change settings, or enter update mode. The identity string is
useful for confirming the installed firmware version before comparing a dump.
"""

from __future__ import annotations

import argparse
import json
import re
import socket
import struct
from pathlib import Path


def query(host: str, port: int, timeout: float) -> str:
    request = b"BJNP" + bytes((0x01, 0x30, 0, 0)) + struct.pack(">HHI", 0, 0, 0)
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.settimeout(timeout)
        sock.sendto(request, (host, port))
        payload, _ = sock.recvfrom(4096)
    if len(payload) < 18 or payload[:4] != b"BJNP" or payload[4] != 0x81 or payload[5] != 0x30:
        raise RuntimeError("unexpected BJNP GetId response")
    length = int.from_bytes(payload[16:18], "big")
    identity = payload[18:18 + length].decode("ascii", "replace")
    return identity.rstrip("\x00")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("host")
    ap.add_argument("--port", type=int, default=8611)
    ap.add_argument("--timeout", type=float, default=3.0)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    identity = query(args.host, args.port, args.timeout)
    fields = dict(re.findall(r"([A-Z]+):([^;]*);", identity))
    result = {"host": args.host, "port": args.port, "identity": identity, "fields": fields}
    text = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
