"""Reads bytes at virtual addresses from a PE or XBE baserom"""

import struct
from pathlib import Path


def _sections(data: bytes) -> list[tuple[int, int, int, int]]:
    if data[:4] == b"XBEH":
        base, headers_size = struct.unpack_from("<II", data, 0x104)
        count, table = struct.unpack_from("<II", data, 0x11C)
        sections = [(base, headers_size, 0, headers_size)]
        for i in range(count):
            sections.append(struct.unpack_from("<IIII", data, table - base + i * 0x38 + 4))
        return sections
    if data[:2] == b"MZ":
        pe = struct.unpack_from("<I", data, 0x3C)[0]
        count, optional_size = struct.unpack_from("<H", data, pe + 6)[0], struct.unpack_from("<H", data, pe + 20)[0]
        optional = pe + 24
        base, headers_size = struct.unpack_from("<I", data, optional + 28)[0], struct.unpack_from("<I", data, optional + 60)[0]
        sections = [(base, headers_size, 0, headers_size)]
        for i in range(count):
            virtual_size, rva, raw_size, raw_offset = struct.unpack_from("<IIII", data, optional + optional_size + i * 40 + 8)
            sections.append((base + rva, virtual_size, raw_offset, raw_size))
        return sections
    raise ValueError("not a PE or XBE")


def read(path: Path, address: int, size: int) -> bytes:
    data = path.read_bytes()
    sections = _sections(data)
    out = bytearray()
    while len(out) < size:
        for start, virtual_size, raw_offset, raw_size in sections:
            offset = address - start
            if 0 <= offset < max(virtual_size, raw_size):
                break
        else:
            raise ValueError(f"{path}: address 0x{address:08x} is not inside any section")
        count = min(size - len(out), max(virtual_size, raw_size) - offset)
        stored = data[raw_offset + offset:raw_offset + min(offset + count, raw_size)] if offset < raw_size else b""
        out += stored + bytes(count - len(stored))
        address += count
    return bytes(out)
