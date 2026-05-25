"""
create_ota.py  —  Package a raw Arduino .bin into a .ota for the UNO R4 WiFi OTA system.

Usage:
    py -3 create_ota.py <input.bin> <output.ota>

Format (from Arduino_ESP32_OTA / uno-r4-wifi-usb-bridge source):
    Offset 0-3  : len     (uint32 LE) = bytes from offset 8 to EOF
    Offset 4-7  : crc32   (uint32 LE) = CRC32 of bytes[8:]
    Offset 8-11 : magic   (uint32 LE) = 0x23411002  (ARDUINO_RA4M1_OTA_MAGIC)
    Offset 12-19: hdr_ver (8 bytes)
    Offset 20+  : LZSS-compressed firmware binary

LZSS parameters (from lzss.cpp decoder):
    EI=11, EJ=4, N=2048, F=17, threshold=2
    Flag bit 1=literal, 0=back-reference
    Bits packed MSB-first into bytes
    Ring buffer initialised with spaces (0x20), start position r = N-F = 2031
"""

import struct, sys, zlib

# ── LZSS parameters ────────────────────────────────────────────────────────────
EI        = 11
EJ        = 4
N         = 1 << EI   # 2048  ring-buffer size
F         = 17         # max match length  (j+2 with j in 0..15)
THRESHOLD = 2          # minimum match length to encode as reference
INIT_CHAR = ord(' ')   # ring buffer fill character

# ── OTA constants ──────────────────────────────────────────────────────────────
MAGIC      = 0x23411002
HDR_VER    = bytes(8)   # 8 zero bytes for hdr_version field


# ── LZSS compressor ────────────────────────────────────────────────────────────

def lzss_compress(data: bytes) -> bytes:
    ring   = bytearray([INIT_CHAR] * N)
    r      = N - F          # write head in ring buffer
    out    = bytearray()
    bit_buf   = 0
    bit_count = 0

    def emit(val: int, n: int):
        nonlocal bit_buf, bit_count
        bit_buf    = (bit_buf << n) | (val & ((1 << n) - 1))
        bit_count += n
        while bit_count >= 8:
            bit_count -= 8
            out.append((bit_buf >> bit_count) & 0xFF)
            bit_buf &= (1 << bit_count) - 1

    pos      = 0
    n_bytes  = len(data)

    # Build a simple index: first_byte → list of ring positions (updated lazily)
    # We search the ring buffer directly; to speed up we skip positions whose
    # first byte doesn't match.

    while pos < n_bytes:
        lookahead_end = min(pos + F, n_bytes)
        lookahead_len = lookahead_end - pos
        first_byte    = data[pos]

        best_len = 0
        best_pos = 0

        if lookahead_len >= THRESHOLD:
            # Search ring buffer using doubled view to handle wrap-around
            doubled = bytes(ring) + bytes(ring)   # length 2*N
            target  = bytes(data[pos:lookahead_end])

            search_start = 0
            while True:
                idx = doubled.find(first_byte.to_bytes(1, 'big'), search_start, N + F)
                if idx == -1 or idx >= N:
                    break
                # Extend match
                length = 0
                while length < len(target) and doubled[idx + length] == target[length]:
                    length += 1
                if length > best_len:
                    best_len = length
                    best_pos = idx    # ring-buffer position (0-based, absolute)
                    if best_len == F:
                        break
                search_start = idx + 1

        if best_len >= THRESHOLD:
            # Encode as back-reference: flag=0, pos(EI bits), j(EJ bits)
            emit(0, 1)
            emit(best_pos & (N - 1), EI)
            emit(best_len - THRESHOLD, EJ)
            for k in range(best_len):
                ring[r] = data[pos + k]
                r = (r + 1) & (N - 1)
            pos += best_len
        else:
            # Encode as literal: flag=1, byte(8 bits)
            emit(1, 1)
            emit(first_byte, 8)
            ring[r] = first_byte
            r = (r + 1) & (N - 1)
            pos += 1

    # Flush remaining bits
    if bit_count > 0:
        out.append((bit_buf << (8 - bit_count)) & 0xFF)

    return bytes(out)


# ── CRC-32 (same table as Arduino utility.cpp) ─────────────────────────────────
# Python's zlib.crc32 matches: init=0, then XOR result with 0 is standard CRC32.
# The Arduino code uses init=0xFFFFFFFF and final XOR 0xFFFFFFFF which is the
# standard ISO-3309 / zlib CRC32.  zlib.crc32() already applies these.

def crc32_arduino(data: bytes) -> int:
    return zlib.crc32(data) & 0xFFFFFFFF


# ── Main ───────────────────────────────────────────────────────────────────────

def create_ota(bin_path: str, ota_path: str):
    raw = open(bin_path, 'rb').read()
    print(f'Input:       {bin_path}  ({len(raw):,} bytes)')

    print('Compressing (LZSS EI=11 EJ=4 N=2048 F=17)...')
    compressed = lzss_compress(raw)
    ratio = len(compressed) / len(raw) * 100
    print(f'Compressed:  {len(compressed):,} bytes  ({ratio:.1f}% of original)')

    # payload = magic + hdr_version + compressed_data
    magic_bytes = struct.pack('<I', MAGIC)
    payload     = magic_bytes + HDR_VER + compressed

    length = len(payload)           # = 4 + 8 + len(compressed)
    crc32  = crc32_arduino(payload)

    header = struct.pack('<II', length, crc32)
    ota    = header + payload

    open(ota_path, 'wb').write(ota)
    print(f'Output:      {ota_path}  ({len(ota):,} bytes)')
    print(f'  len=0x{length:08X}  crc32=0x{crc32:08X}  magic=0x{MAGIC:08X}')


if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('Usage: py -3 create_ota.py <input.bin> <output.ota>')
        sys.exit(1)
    create_ota(sys.argv[1], sys.argv[2])
