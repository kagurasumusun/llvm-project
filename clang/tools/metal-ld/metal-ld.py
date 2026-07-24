#!/usr/bin/env python3
"""metal-ld: Link multiple .air files into .metallib.

This tool takes multiple .air files as input and produces a .metallib file
by creating a Mach-O universal binary container with MTLB sections.
"""

import argparse
import hashlib
import struct
import sys
from pathlib import Path


# Mach-O universal binary constants
FAT_MAGIC = 0xCAFEBABE
FAT_CIGAM = 0xBEBAFECA  # Byte-swapped

# Section types
SECTION_NAME = b'NAME'
SECTION_TYPE = b'TYPE'
SECTION_HASH = b'HASH'
SECTION_XSOFFT = b'XSOFFT'
SECTION_VERS = b'VERS'
SECTION_MDSZ = b'MDSZ'
SECTION_ENDT = b'ENDT'


def create_section(name: bytes, data: bytes) -> bytes:
    """Create a metallib section with header."""
    # Section format:
    # - 4 bytes: section name (e.g., "NAME", "TYPE")
    # - 4 bytes: padding
    # - 8 bytes: data size
    # - Variable: data
    
    section = bytearray()
    section.extend(name)
    section.extend(b'\x00' * (4 - len(name)))  # Padding to 4 bytes
    section.extend(struct.pack('<Q', len(data)))
    section.extend(data)
    
    # Align to 8 bytes
    padding = (8 - (len(section) % 8)) % 8
    section.extend(b'\x00' * padding)
    
    return bytes(section)


def create_name_section(name: str) -> bytes:
    """Create NAME section with function/object name."""
    return create_section(SECTION_NAME, name.encode('utf-8') + b'\x00')


def create_type_section(type_name: str) -> bytes:
    """Create TYPE section with type information."""
    return create_section(SECTION_TYPE, type_name.encode('utf-8') + b'\x00')


def create_hash_section(data: bytes) -> bytes:
    """Create HASH section with SHA256 hash."""
    hash_value = hashlib.sha256(data).digest()
    return create_section(SECTION_HASH, hash_value)


def create_version_section(version: str = '32023.883') -> bytes:
    """Create VERS section with compiler version."""
    return create_section(SECTION_VERS, version.encode('utf-8') + b'\x00')


def create_mdsz_section(size: int) -> bytes:
    """Create MDSZ (metadata size) section."""
    return create_section(SECTION_MDSZ, struct.pack('<Q', size))


def create_xsofft_section(offset: int) -> bytes:
    """Create XSOFFT (offset) section."""
    return create_section(SECTION_XSOFFT, struct.pack('<Q', offset))


def create_endt_section() -> bytes:
    """Create ENDT (end) section."""
    return create_section(SECTION_ENDT, b'')


def create_fat_header(num_archs: int, arch_sizes: list) -> bytes:
    """Create Mach-O universal binary (fat) header."""
    header = bytearray()
    
    # Magic number (big-endian)
    header.extend(struct.pack('>I', FAT_MAGIC))
    
    # Number of architectures
    header.extend(struct.pack('>I', num_archs))
    
    # Architecture descriptors (each 20 bytes)
    offset = 8 + (num_archs * 20)  # Start after header + descriptors
    for i, size in enumerate(arch_sizes):
        # CPU type (example: 0x01000007 for x86_64)
        header.extend(struct.pack('>I', 0x01000007))
        # CPU subtype
        header.extend(struct.pack('>I', 0x00000003))
        # Offset
        header.extend(struct.pack('>I', offset))
        # Size
        header.extend(struct.pack('>I', size))
        # Alignment (2^4 = 16 bytes)
        header.extend(struct.pack('>I', 4))
        
        # Align offset to 16 bytes
        offset += size
        offset = (offset + 15) & ~15
    
    return bytes(header)


def link_metallib(air_files: list, output_path: Path, name: str = 'metal_library') -> None:
    """Link multiple .air files into .metallib."""
    
    # Read all .air files
    air_data_list = []
    for air_file in air_files:
        data = air_file.read_bytes()
        air_data_list.append((air_file.name, data))
    
    # Create sections for each .air file
    sections_list = []
    for air_name, air_data in air_data_list:
        sections = bytearray()
        
        # Add sections
        sections.extend(create_name_section(name))
        sections.extend(create_type_section('air'))
        sections.extend(create_hash_section(air_data))
        sections.extend(create_xsofft_section(len(sections) + 8 * 6))  # Offset to bitcode
        sections.extend(create_version_section())
        sections.extend(create_mdsz_section(len(air_data)))
        sections.extend(air_data)
        sections.extend(create_endt_section())
        
        sections_list.append(bytes(sections))
    
    # Create fat header
    arch_sizes = [len(s) for s in sections_list]
    fat_header = create_fat_header(len(sections_list), arch_sizes)
    
    # Write .metallib file
    with open(output_path, 'wb') as f:
        f.write(fat_header)
        
        # Write each section with alignment
        offset = len(fat_header)
        for sections in sections_list:
            # Align to 16 bytes
            padding = (16 - (offset % 16)) % 16
            f.write(b'\x00' * padding)
            f.write(sections)
            offset += padding + len(sections)
    
    total_size = output_path.stat().st_size
    print(f"Created {output_path} ({total_size} bytes) with {len(air_files)} .air file(s)")


def main() -> None:
    parser = argparse.ArgumentParser(description='Link .air files into .metallib')
    parser.add_argument('inputs', type=Path, nargs='+', help='Input .air files')
    parser.add_argument('-o', '--output', type=Path, required=True, help='Output .metallib file')
    parser.add_argument('--name', default='metal_library', help='Library name (default: metal_library)')
    
    args = parser.parse_args()
    
    # Verify all input files exist
    for input_file in args.inputs:
        if not input_file.exists():
            print(f"Error: Input file {input_file} not found", file=sys.stderr)
            sys.exit(1)
    
    link_metallib(args.inputs, args.output, args.name)


if __name__ == '__main__':
    main()
