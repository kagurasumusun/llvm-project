#!/usr/bin/env python3
"""Extract the real arity of every `__metal_*` builtin from Apple's stdlib.

The builtins are never declared in the headers -- they are provided by the
compiler -- so the only way to learn their signatures from the reference set is
to read every call site in <metal_stdlib> and count the arguments. This matters
because declaring them all as the generic `"v."` (void, variadic) makes CodeGen
crash on any call whose result is used.

Usage: extract_builtin_arity.py <path to include/metal> > builtin_arity.csv
"""
import os, re, sys, collections

ROOT = sys.argv[1]

CALL = re.compile(r'\b(__metal_[A-Za-z0-9_]+)\s*\(')


def split_args(src, i):
    """Return the argument list of a call whose '(' is at src[i], or None."""
    depth = 0
    args = []
    cur = []
    n = len(src)
    while i < n:
        c = src[i]
        if c in "([{":
            depth += 1
            if depth == 1:
                i += 1
                continue
        elif c in ")]}":
            depth -= 1
            if depth == 0:
                args.append("".join(cur).strip())
                return [a for a in args if a != ""]
        elif c == "," and depth == 1:
            args.append("".join(cur).strip())
            cur = []
            i += 1
            continue
        elif c == '"':  # skip string literals
            j = i + 1
            while j < n and src[j] != '"':
                j += 2 if src[j] == "\\" else 1
            cur.append(src[i:j + 1])
            i = j + 1
            continue
        if depth >= 1:
            cur.append(c)
        i += 1
    return None


arity = collections.defaultdict(collections.Counter)
examples = {}

for dirpath, _, files in os.walk(ROOT):
    for fn in files:
        p = os.path.join(dirpath, fn)
        try:
            src = open(p, errors="replace").read()
        except Exception:
            continue
        # strip comments so commented-out calls do not pollute the counts
        src = re.sub(r'/\*.*?\*/', ' ', src, flags=re.S)
        src = re.sub(r'//[^\n]*', ' ', src)
        for m in CALL.finditer(src):
            name = m.group(1)
            args = split_args(src, m.end() - 1)
            if args is None:
                continue
            arity[name][len(args)] += 1
            examples.setdefault((name, len(args)),
                                m.group(0) + ", ".join(args) + ")")

print("builtin,arity,occurrences,all_arities,example")
for name in sorted(arity):
    c = arity[name]
    best, n = c.most_common(1)[0]
    alls = ";".join("%d:%d" % kv for kv in sorted(c.items()))
    ex = examples[(name, best)].replace("\n", " ")
    ex = re.sub(r"\s+", " ", ex)[:160]
    print('%s,%d,%d,%s,"%s"' % (name, best, n, alls, ex.replace('"', "'")))
