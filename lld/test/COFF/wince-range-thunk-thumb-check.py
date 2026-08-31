#!/usr/bin/env python3
"""Assert the Thumb veneer literal is the callee VA with bit 0, not 0."""
import re
import sys

nm = open(sys.argv[1], encoding="utf-8").read()
dump = open(sys.argv[2], encoding="utf-8").read()
m = re.search(r"([0-9a-fA-F]+)\s+[A-Za-z]\s+callee\b", nm)
if not m:
    sys.exit("callee missing from llvm-nm:\n" + nm)
addr = int(m.group(1), 16)
if addr == 0:
    sys.exit("callee address is 0")
hexflat = re.sub(r"[^0-9a-fA-F]", "", dump).lower()
cands = [addr, addr | 1, addr + 0x10000, (addr + 0x10000) | 1]
found = None
for c in cands:
    if c.to_bytes(4, "little").hex() in hexflat:
        found = c
        break
if found is None:
    sys.exit(f"callee {addr:#x} (or |1 / +imagebase) not in .text:\n{dump}")
if (found & 1) == 0:
    sys.exit(f"literal {found:#x} is even; Thumb dest must have bit 0")
if found <= 1:
    sys.exit("placeholder Absolute 0")
print(f"ok literal {found:#x}")
