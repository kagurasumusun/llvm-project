#!/usr/bin/env python3
"""research/datasets の全 CSV を 1 行も飛ばさずフルロードし、要点を抽出する。

対象:
  rtlib_cleanroom_map.csv / rtlib_metal_only_map.csv / stdlib_exhaustive_behavioral_map.csv
  rtlib_members.csv / rtlib_layer_map.csv / rtlib_pairing.csv
  builtin_to_air_map.v2.csv ほか datasets 配下の全 CSV(.gz 含む)

出力: docs-metal/data/fullscan/csv_*.csv, csv_summary.json
"""
import os, sys, csv, gzip, json, collections, re

DS = sys.argv[1] if len(sys.argv) > 1 else "/tmp/metal-info/research/datasets"
OUT = sys.argv[2] if len(sys.argv) > 2 else "/home/user/llvm-project/docs-metal/data/fullscan"
os.makedirs(OUT, exist_ok=True)
csv.field_size_limit(1 << 30)

def opener(p):
    return gzip.open(p, "rt", errors="replace") if p.endswith(".gz") else open(p, "r", errors="replace")

summary = {}
files = []
for dirpath, _, fns in os.walk(DS):
    for fn in fns:
        if fn.endswith(".csv") or fn.endswith(".csv.gz"):
            files.append(os.path.join(dirpath, fn))
files.sort()

# 汎用: 各ファイルの全行を読み、列ごとの値集合を集計（高カーディナリティ列は上位のみ）
col_stats = {}
for p in files:
    rel = os.path.relpath(p, DS)
    try:
        with opener(p) as f:
            rd = csv.reader(f)
            try:
                hdr = next(rd)
            except StopIteration:
                continue
            ctrs = [collections.Counter() for _ in hdr]
            n = 0
            for row in rd:
                n += 1
                for i, v in enumerate(row[:len(hdr)]):
                    ctrs[i][v[:300]] += 1
            col_stats[rel] = (hdr, ctrs, n)
            summary[rel] = {"rows": n, "cols": len(hdr),
                            "col_cardinality": {h: len(c) for h, c in zip(hdr, ctrs)}}
            print("%-50s rows=%-8d cols=%d" % (rel, n, len(hdr)), flush=True)
    except Exception as e:
        summary[rel] = {"error": str(e)}
        print("ERR", rel, e, flush=True)

# 低カーディナリティ列(<=400)は全値を出力。高カーディナリティ列は上位200。
with open(os.path.join(OUT, "csv_column_values.txt"), "w") as out:
    for rel in sorted(col_stats):
        hdr, ctrs, n = col_stats[rel]
        out.write("\n########## %s (rows=%d) ##########\n" % (rel, n))
        for h, c in zip(hdr, ctrs):
            out.write("\n=== column: %s  (distinct=%d) ===\n" % (h, len(c)))
            items = c.most_common() if len(c) <= 400 else c.most_common(200)
            for k, v in items:
                out.write("%8d  %s\n" % (v, k.replace("\n", "\\n")))
            if len(c) > 400:
                out.write("... (%d more distinct values)\n" % (len(c) - 200))

# 専用抽出: air 組み込み名とガードマクロの悉皆
air_names = collections.Counter()
guards = collections.Counter()
builtins = collections.Counter()
ulps = collections.Counter()
re_air = re.compile(r'\bair\.[\w.]*')
re_guard = re.compile(r'__HAVE_[A-Z0-9_]+__')
re_metal = re.compile(r'\b__metal_[\w]+')
for p in files:
    with opener(p) as f:
        for line in f:
            for m in re_air.findall(line):
                air_names[m] += 1
            for m in re_guard.findall(line):
                guards[m] += 1
            for m in re_metal.findall(line):
                builtins[m] += 1

for nm, c in [("air_names", air_names), ("have_guards", guards), ("metal_builtins", builtins)]:
    with open(os.path.join(OUT, "csv_%s.csv" % nm), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["value", "count"])
        for k, v in c.most_common():
            w.writerow([k, v])

summary["_totals"] = {"files": len(files),
                      "air_names": len(air_names),
                      "have_guards": len(guards),
                      "metal_builtins": len(builtins)}
json.dump(summary, open(os.path.join(OUT, "csv_summary.json"), "w"), indent=2)
print(json.dumps(summary["_totals"], indent=2))
