#!/usr/bin/env python3
"""全 .ll から !air.* を含む生メタデータ行（数値オペランド込み）を悉皆収集し、
!N 番号と関数ポインタ名を正規化して重複排除する。

出力: docs-metal/data/fullscan/ir_airmd_raw.txt
"""
import os, re, sys, collections

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/metal-info"
OUT = sys.argv[2] if len(sys.argv) > 2 else "/home/user/llvm-project/docs-metal/data/fullscan"
os.makedirs(OUT, exist_ok=True)

re_line = re.compile(r'^!\d+ = (?:distinct )?!\{(.*)\}\s*$')
re_num = re.compile(r'!\d+')

groups = collections.defaultdict(collections.Counter)
for dirpath, _, fns in os.walk(ROOT):
    for fn in fns:
        if not fn.endswith(".ll"):
            continue
        for line in open(os.path.join(dirpath, fn), errors="replace"):
            if not line.startswith("!") or '"air.' not in line:
                continue
            m = re_line.match(line.rstrip())
            if not m:
                continue
            body = re_num.sub("!N", m.group(1))
            head = re.search(r'!"(air\.[\w.]*)"', body)
            groups[head.group(1) if head else "?"][body] += 1

with open(os.path.join(OUT, "ir_airmd_raw.txt"), "w") as f:
    for head in sorted(groups, key=lambda h: -sum(groups[h].values())):
        c = groups[head]
        f.write("\n########## %s  (total=%d, variants=%d) ##########\n"
                % (head, sum(c.values()), len(c)))
        for body, n in c.most_common():
            f.write("%8d  !{%s}\n" % (n, body))
print("heads:", len(groups), "variants:", sum(len(c) for c in groups.values()))
