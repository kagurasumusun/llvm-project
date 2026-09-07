#!/usr/bin/env python3
"""Assert the Thumb veneer literal is the callee VA with bit 0, not 0.

The callee address comes from the export table (a stripped PE has no
symbol table): the export RVA carries the Thumb bit, and the veneer
literal is the corresponding VA (RVA + image base) with the same bit.
"""
import re
import sys

exports = open(sys.argv[1], encoding="utf-8").read()
dump = open(sys.argv[2], encoding="utf-8").read()
m = re.search(r"Name:\s+callee\s*\n\s*RVA:\s*0x([0-9a-fA-F]+)", exports)
if not m:
    sys.exit("callee missing from export table:\n" + exports)
rva = int(m.group(1), 16)
if rva == 0:
    sys.exit("callee RVA is 0")
hexflat = re.sub(r"[^0-9a-fA-F]", "", dump).lower()
cands = [rva, rva | 1, rva + 0x10000, (rva + 0x10000) | 1]
found = None
for c in cands:
    if c.to_bytes(4, "little").hex() in hexflat:
        found = c
        break
if found is None:
    sys.exit(f"callee {rva:#x} (or |1 / +imagebase) not in .text:\n{dump}")
if (found & 1) == 0:
    sys.exit(f"literal {found:#x} is even; Thumb dest must have bit 0")
if found <= 1:
    sys.exit("placeholder Absolute 0")
print(f"ok literal {found:#x}")
