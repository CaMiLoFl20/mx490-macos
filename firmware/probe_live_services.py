#!/usr/bin/env python3
"""Probe read-only Canon HTTP routes relevant to native AirScan support."""

from __future__ import annotations

import argparse
import http.client
import json
import ssl
from pathlib import Path


ROUTES = (
    "/eSCL/ScannerCapabilities",
    "/eSCL/ScannerStatus",
    "/wsd/",
    "/index.html",
    "/English/pages_WinUS/firm_update.cgi",
)


def probe(host: str, port: int, use_tls: bool, path: str, timeout: float) -> dict[str, object]:
    context = ssl._create_unverified_context() if use_tls else None
    connection = (http.client.HTTPSConnection(host, port, timeout=timeout, context=context)
                  if use_tls else http.client.HTTPConnection(host, port, timeout=timeout))
    try:
        connection.request("GET", path)
        response = connection.getresponse()
        response.read()
        return {
            "scheme": "https" if use_tls else "http",
            "path": path,
            "status": response.status,
            "server": response.getheader("Server", ""),
            "content_type": response.getheader("Content-Type", ""),
        }
    except OSError as exc:
        return {"scheme": "https" if use_tls else "http", "path": path,
                "error": str(exc)}
    finally:
        connection.close()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("host")
    ap.add_argument("--timeout", type=float, default=4.0)
    ap.add_argument("-o", "--output", type=Path)
    args = ap.parse_args()
    results = []
    for tls, port in ((False, 80), (True, 443)):
        results.extend(probe(args.host, port, tls, path, args.timeout)
                       for path in ROUTES)
    payload = {"host": args.host, "routes": results}
    text = json.dumps(payload, indent=2) + "\n"
    if args.output:
        args.output.write_text(text)
    print(text, end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
