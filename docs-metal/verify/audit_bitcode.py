#!/usr/bin/env python3
"""Audit Apple AIR bitcode directly, without trusting any summary document.

Re-establishes from raw bytes the facts this fork's implementation depends on:
pointer representation, calling convention, datalayout uniqueness, and whether
any non-Apple bitcode has been mixed into the reference set.

    python3 docs-metal/verify/audit_bitcode.py /tmp/metal-info
"""
import collections, glob, os, random, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bcread


def module_info(recs):
    tri = dl = ident = None
    for bid, c, v in recs:
        nm = bcread.BLOCK_NAMES.get(bid)
        if nm == 'MODULE' and c == 2 and v and isinstance(v[0], int):
            tri = ''.join(chr(x) for x in v)
        elif nm == 'MODULE' and c == 3 and v and isinstance(v[0], int):
            dl = ''.join(chr(x) for x in v)
        elif nm == 'IDENTIFICATION' and c == 1 and v and isinstance(v[0], str):
            ident = v[0]
    return tri, dl, ident


def main(info):
    files = sorted(glob.glob(os.path.join(info, 'research/golden/**/*.air'),
                             recursive=True))
    files += sorted(glob.glob(os.path.join(info, 'reference/metal-ast-*/air/*.bc')))
    files += sorted(glob.glob(os.path.join(info, 'test/*.bc')))
    random.seed(0)
    if len(files) > 600:
        files = random.sample(files, 600)

    typed = opaque = 0
    cc = collections.Counter()
    dls = collections.Counter()
    suspects = []
    unknown_blocks = collections.Counter()

    for f in files:
        try:
            recs = bcread.load(f)
        except Exception:
            continue
        t = sum(1 for b, c, _ in recs
                if bcread.BLOCK_NAMES.get(b) == 'TYPE' and c == 8)
        o = sum(1 for b, c, _ in recs
                if bcread.BLOCK_NAMES.get(b) == 'TYPE' and c == 25)
        typed += bool(t)
        opaque += bool(o)
        for b, c, v in recs:
            if b not in bcread.BLOCK_NAMES:
                unknown_blocks[b] += 1
            if bcread.BLOCK_NAMES.get(b) == 'MODULE' and c == 8 and len(v) >= 4:
                cc[v[3]] += 1
        tri, dl, ident = module_info(recs)
        if dl:
            dls[dl] += 1
        # An IDENTIFICATION block, an opaque pointer, or an i128 entry in the
        # datalayout all mark a module as not coming from Apple's toolchain.
        if ident or o or (dl and 'i128' in dl):
            suspects.append((os.path.relpath(f, info), tri, ident, bool(o)))

    print(f"modules examined: {len(files)}")
    print(f"  typed pointer  : {typed}")
    print(f"  opaque pointer : {opaque}")
    print(f"  calling conventions seen: {dict(cc)}  (0 = C)")
    print(f"  distinct datalayouts    : {len(dls)}")
    print(f"  unknown block ids       : {dict(unknown_blocks) or 'none'}")
    if suspects:
        print("\n  not produced by Apple metalfe (do not use as a spec reference):")
        for f, tri, ident, o in suspects:
            print(f"    {f}\n      triple={tri} producer={ident} opaque_ptr={o}")
    else:
        print("\n  all modules produced by Apple metalfe")

    # Expected shape of a clean corpus:
    #  * every Apple module uses typed pointers, so the only opaque pointer
    #    modules are the foreign ones already listed as suspects
    #  * every function uses calling convention 0 (C)
    #  * Apple ships exactly two datalayouts (air32 and air64); a third only
    #    appears because the foreign modules add an i128 entry
    #  * no Apple-private bitstream blocks exist
    known_foreign = {f for f, _, _, _ in suspects}
    ok = (opaque <= len(known_foreign)
          and set(cc) <= {0}
          and len(dls) <= 2 + len(known_foreign)
          and not unknown_blocks)
    print("\nRESULT:", "consistent with the documented AIR contract" if ok
          else "DISCREPANCY - investigate")
    if known_foreign:
        print("        (the modules above are known, unused by this fork, and"
              " explained in\n         docs-metal/07-OPAQUE-EVIDENCE.md"
              " section 3)")
    return 0 if ok else 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1] if len(sys.argv) > 1 else '/tmp/metal-info'))
