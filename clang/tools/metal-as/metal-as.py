#!/usr/bin/env python3
"""metal-as: Assemble LLVM bitcode into .air files with MTLB header.

This tool takes LLVM bitcode (.bc) as input and produces .air files
by adding the MTLB header that Apple's Metal compiler uses.
"""

import argparse
import struct
import sys
from pathlib import Path


def create_mtlb_header(bitcode_size: int, triple: str) -> bytes:
    """Create MTLB header for .air file.
    
    MTLB header structure (284 bytes):
    - Magic: "MTLB" (4 bytes)
    - Version/flags (4 bytes)
    - Various fields (276 bytes)
    """
    header = bytearray(284)
    
    # Magic number
    header[0:4] = b'MTLB'
    
    # Version/flags (example: 0x00028001)
    struct.pack_into('<I', header, 4, 0x00028001)
    
    # File size field (offset 16-23)
    total_size = len(header) + bitcode_size + len(triple) + 1
    struct.pack_into('<Q', header, 16, total_size)
    
    # Other fields (based on reverse engineering)
    struct.pack_into('<Q', header, 8, 4)  # Some count/flag
    struct.pack_into('<Q', header, 24, 0x82)  # Some offset/size
    struct.pack_into('<Q', header, 32, 0xf8)
    struct.pack_into('<Q', header, 40, 8)
    struct.pack_into('<Q', header, 48, 0x100)
    
    return bytes(header)


def assemble_air(bitcode_path: Path, output_path: Path, triple: str) -> None:
    """Assemble .bc file into .air file with MTLB header."""
    
    # Read LLVM bitcode
    bitcode = bitcode_path.read_bytes()
    
    # Verify it's LLVM bitcode
    if bitcode[:4] != b'BC\xc0\xde':
        print(f"Warning: {bitcode_path} does not appear to be LLVM bitcode", file=sys.stderr)
    
    # Create MTLB header
    header = create_mtlb_header(len(bitcode), triple)
    
    # Write .air file
    with open(output_path, 'wb') as f:
        f.write(header)
        f.write(bitcode)
        # Append triple string (null-terminated)
        f.write(triple.encode('utf-8'))
        f.write(b'\x00')
    
    print(f"Created {output_path} ({len(header) + len(bitcode) + len(triple) + 1} bytes)")


def main() -> None:
    parser = argparse.ArgumentParser(description='Assemble LLVM bitcode into .air files')
    parser.add_argument('input', type=Path, help='Input LLVM bitcode file (.bc)')
    parser.add_argument('-o', '--output', type=Path, required=True, help='Output .air file')
    parser.add_argument('--triple', default='air64-apple-macosx10.15.0',
                       help='Target triple (default: air64-apple-macosx10.15.0)')
    
    args = parser.parse_args()
    
    if not args.input.exists():
        print(f"Error: Input file {args.input} not found", file=sys.stderr)
        sys.exit(1)
    
    assemble_air(args.input, args.output, args.triple)


if __name__ == '__main__':
    main()
