#!/usr/bin/env python3
"""
audit-coredll.py - verify the vendored COREDLL import surface against a
real device's export list.

Usage:
    dumpbin /EXPORTS coredll.dll > coredll.txt      # on a Windows host,
    # or copy CoreDLL.dll from the device (\Windows) and dump it
    python3 audit-coredll.py coredll.txt [coredll6.txt]

Compares every export name in the dump(s) against
wince-sysroot/mingwrt/{coredll.def,coredll6.def} (+ the w32api copy) and
prints:
  * exports missing from the def files (add these - def-only change),
  * def entries not present in the dump (candidates for removal; CE OEM
    variation means a def name missing from ONE device's dump is not
    necessarily wrong - cross-check several devices/generations).

The 2010-era CE5/WM6 dump used for the initial audit is archived at
https://www.cnblogs.com/lucienbao/archive/2010/10/29/wince_coredll.html
(1799 functions); chunks 0/1/4/7 (565 names) were verified against the
vendored defs with only 30 gaps, all added.  A dump from YOUR device is
the authoritative source for OEM-specific surfaces.
"""

import sys, os

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", ".."))
DEFS = (os.path.join(REPO, "wince-sysroot/mingwrt/coredll.def"),
        os.path.join(REPO, "wince-sysroot/mingwrt/coredll6.def"),
        os.path.join(REPO, "wince-sysroot/w32api/libce/coredll.def"))


def def_names(paths):
    names = set()
    for p in paths:
        for line in open(p, encoding="latin1"):
            line = line.strip()
            if line and not line.startswith(";") \
               and not line.upper().startswith("LIBRARY") \
               and not line.upper().startswith("EXPORTS") \
               and " " not in line:
                names.add(line)
    return names


def dump_names(path):
    """Parse `dumpbin /EXPORTS` output (ordinal + name lines, or .def-style
    plain name lists)."""
    names = set()
    started = False
    for line in open(path, encoding="latin1", errors="replace"):
        line = line.strip()
        if line.upper().startswith("EXPORTS") or line == "ordinal    name":
            started = True
            continue
        if not started or not line:
            continue
        toks = line.split()
        # dumpbin: "<ordinal> <hint> <rva> <name>" or "<ordinal> <name>"
        if toks[0].isdigit():
            if len(toks) >= 2 and not toks[1].isdigit():
                names.add(toks[-1] if len(toks) >= 4 else toks[1])
            continue
        if len(toks) == 1 and not toks[0][0].isdigit():
            names.add(toks[0])
    return names


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    defs = def_names(DEFS)
    dumps = set()
    for arg in sys.argv[1:]:
        dumps |= dump_names(arg)
    missing = sorted(dumps - defs)
    extra = sorted(defs - dumps)
    print(f"dump exports: {len(dumps)}   def entries: {len(defs)}")
    print(f"\n== MISSING from def files ({len(missing)}):")
    for n in missing:
        print("  ", n)
    print(f"\n== def entries not in dump ({len(extra)}) - OEM variation;")
    print("   cross-check before removing:")
    for n in extra:
        print("  ", n)
    sys.exit(1 if missing else 0)


if __name__ == "__main__":
    main()
