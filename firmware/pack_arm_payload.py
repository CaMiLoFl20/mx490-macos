#!/usr/bin/env python3
"""Resolve the small ARM/Thumb relocatable eSCL probe without a linker.

This intentionally supports only the relocation types emitted by the probe:
R_ARM_REL32 (3) and R_ARM_THM_CALL (10). It emits a flat text+rodata blob and
never edits a Canon firmware image.
"""
from __future__ import annotations
import argparse, json, struct
from pathlib import Path

SHF_ALLOC = 2
SHT_PROGBITS = 1
SHT_NOBITS = 8
R_ARM_REL32 = 3
R_ARM_THM_CALL = 10

def cstr(buf: bytes, off: int) -> str:
    end = buf.find(bytes([0]), off)
    return buf[off:end].decode("ascii", "replace")

def parse(path: Path):
    b = path.read_bytes()
    if b[:4] != b"\x7fELF" or b[4] != 1 or b[5] != 1:
        raise ValueError("expected 32-bit little-endian ELF")
    h = struct.unpack_from("<16sHHIIIIIHHHHHH", b, 0)
    shoff, shents, shnum, shstr = h[6], h[11], h[12], h[13]
    sections = [struct.unpack_from("<IIIIIIIIII", b, shoff + i * shents) for i in range(shnum)]
    names_sec = sections[shstr]
    names = b[names_sec[4]:names_sec[4] + names_sec[5]]
    names_by_idx = {i: cstr(names, s[0]) for i, s in enumerate(sections)}
    sec_by_name = {name: i for i, name in names_by_idx.items()}
    sym_idx = sec_by_name[".symtab"]
    symsec = sections[sym_idx]
    strtab = b[sections[symsec[6]][4]:sections[symsec[6]][4] + sections[symsec[6]][5]]
    symbols = []
    for off in range(symsec[4], symsec[4] + symsec[5], symsec[9]):
        st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack_from("<IIIBBH", b, off)
        symbols.append({"name": cstr(strtab, st_name), "value": st_value, "size": st_size, "shndx": st_shndx})
    return b, sections, names_by_idx, symbols

def thumb_call(blob: bytearray, off: int, pc: int, target: int) -> None:
    # Thumb BL uses PC=(instruction address+4), with a signed 25-bit offset.
    delta = target - (pc + 4)
    if delta & 1 or delta < -(1 << 24) or delta >= (1 << 24):
        raise ValueError(f"Thumb call out of range: {hex(pc)} -> {hex(target)}")
    imm = delta & 0x01FFFFFE
    s = (imm >> 24) & 1
    i1 = (imm >> 23) & 1
    i2 = (imm >> 22) & 1
    j1 = (~(i1 ^ s)) & 1
    j2 = (~(i2 ^ s)) & 1
    hw1 = 0xF000 | (s << 10) | ((imm >> 12) & 0x3FF)
    hw2 = 0xD000 | (j1 << 13) | (j2 << 11) | ((imm >> 1) & 0x7FF)
    struct.pack_into("<HH", blob, off, hw1, hw2)

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("object", type=Path)
    ap.add_argument("--base", required=True, type=lambda x: int(x, 0))
    ap.add_argument("-o", "--output", type=Path, required=True)
    ap.add_argument("--manifest", type=Path)
    args = ap.parse_args()
    raw, sections, names, symbols = parse(args.object)
    wanted = [".text", ".rodata.str1.1", ".rodata"]
    layout = {}
    cursor = 0
    blob = bytearray()
    for name in wanted:
        idx = next((i for i, n in names.items() if n == name), None)
        if idx is None:
            continue
        sec = sections[idx]
        align = max(1, sec[8])
        cursor = (cursor + align - 1) & ~(align - 1)
        if len(blob) < cursor: blob.extend(bytes([0]) * (cursor - len(blob)))
        layout[idx] = cursor
        size = sec[5]
        if sec[1] == SHT_NOBITS: data = bytes(size)
        else: data = raw[sec[4]:sec[4] + size]
        blob.extend(data)
        cursor += size
    # Resolve relocations against the flattened section layout.
    for idx, name in names.items():
        if not name.startswith(".rel."): continue
        target_name = name[5:]
        if target_name != ".text": continue
        sec = sections[idx]
        for roff in range(sec[4], sec[4] + sec[5], sec[9]):
            rel_off, info = struct.unpack_from("<II", raw, roff)
            sym_no, typ = info >> 8, info & 0xFF
            if typ not in (R_ARM_REL32, R_ARM_THM_CALL):
                raise ValueError(f"unsupported relocation type {typ}")
            sym = symbols[sym_no]
            if sym["shndx"] not in layout:
                raise ValueError(f"symbol {sym['name']} is outside packed sections")
            s = args.base + layout[sym["shndx"]] + sym["value"]
            p = args.base + layout[sections.index(next(s for i, s in enumerate(sections) if names.get(i) == target_name))] + rel_off
            out_off = p - args.base
            if typ == R_ARM_REL32:
                addend = struct.unpack_from("<I", blob, out_off)[0]
                struct.pack_into("<I", blob, out_off, (s + addend - p) & 0xFFFFFFFF)
            else:
                thumb_call(blob, out_off, p, s)
    args.output.write_bytes(blob)
    manifest = {"object": str(args.object), "base": hex(args.base), "size": len(blob), "sections": {names[i]: {"offset": off, "size": sections[i][5]} for i, off in layout.items()}, "relocations": [R_ARM_REL32, R_ARM_THM_CALL], "flashable": False}
    if args.manifest: args.manifest.write_text(json.dumps(manifest, indent=2) + "\n")
    print(json.dumps(manifest, indent=2))
    return 0

if __name__ == "__main__": raise SystemExit(main())
