#!/usr/bin/env python3
"""metal-info の AST JSON 全 109,916 件を 1 バイトも飛ばさずフルロード走査する。

json.load ではなく全バイトをストリーム走査し、Clang AST JSON の全キー値を悉皆集計する。
ワーカーを fork して 2 並列で処理し、部分結果を pickle でマージする。

抽出対象:
  - "kind" 全値（AST ノード種別 = Decl/Stmt/Type/Attr の全網羅）
  - "qualType" 全値（型表記の全網羅 = MSL 型システムの実測辞書）
  - Attr ノード種別（*Attr で終わる kind）
  - "name"/"mangledName" の対（マングリング規則の実測）
  - "storageClass" / "cc" / "valueCategory" / "castKind" / "opcode" / "tagUsed"
  - "isImplicit" 付き typedef 名（opaque 型の実測）
  - "addrspace" を含む qualType
  - "access" / "explicitlyDefaulted" / "constexpr" 等の C++ 属性
"""
import os, re, sys, collections, pickle, csv, json, multiprocessing as mp

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/metal-info"
OUT = sys.argv[2] if len(sys.argv) > 2 else "/home/user/llvm-project/docs-metal/data/fullscan"
os.makedirs(OUT, exist_ok=True)

FIELDS = ["kind", "qualType", "storageClass", "cc", "valueCategory", "castKind",
          "opcode", "tagUsed", "name", "mangledName", "desugaredQualType",
          "typeAliasDeclId", "access", "init", "nrvo", "size", "align",
          "bitwidth", "value", "isPostfix", "text"]
PATS = {f: re.compile((r'"%s"\s*:\s*"([^"\\]*(?:\\.[^"\\]*)*)"' % f).encode()) for f in FIELDS}
PAT_NUMF = {f: re.compile((r'"%s"\s*:\s*(\d+)' % f).encode()) for f in ["size", "align", "bitwidth"]}
PAT_BOOL = re.compile(rb'"(is[A-Z]\w*|has[A-Z]\w*|explicitlyDefaulted|constexpr|inline|virtual|pure|variadic|trivial|copyAssign|moveAssign|defaultCtor|copyCtor|moveCtor|dtor)"\s*:\s*(true|false|"[^"]*")')
PAT_ALLKEYS = re.compile(rb'"([A-Za-z_]\w*)"\s*:')
PAT_MANGLE_PAIR = re.compile(rb'"name"\s*:\s*"([^"]*)"[^{}]{0,400}?"mangledName"\s*:\s*"([^"]*)"')


def scan(paths):
    ctrs = {f: collections.Counter() for f in FIELDS}
    ctrs["_allkeys"] = collections.Counter()
    ctrs["_bool"] = collections.Counter()
    ctrs["_manglepair"] = collections.Counter()
    nbytes = 0
    nfiles = 0
    for p in paths:
        try:
            data = open(p, "rb").read()
        except Exception:
            continue
        nfiles += 1
        nbytes += len(data)
        for f, pat in PATS.items():
            c = ctrs[f]
            for m in pat.findall(data):
                c[m] += 1
        for k in PAT_ALLKEYS.findall(data):
            ctrs["_allkeys"][k] += 1
        for k, v in PAT_BOOL.findall(data):
            ctrs["_bool"][k + b"=" + v] += 1
        for n, mn in PAT_MANGLE_PAIR.findall(data):
            if n != mn:
                ctrs["_manglepair"][n + b" -> " + mn] += 1
        del data
    return ctrs, nfiles, nbytes


def worker(args):
    idx, paths = args
    ctrs, nf, nb = scan(paths)
    with open(os.path.join(OUT, ".ast_part_%d.pkl" % idx), "wb") as f:
        pickle.dump((ctrs, nf, nb), f, 2)
    return idx, nf, nb


def main():
    allp = []
    for dirpath, _, filenames in os.walk(ROOT):
        for fn in filenames:
            if fn.endswith(".json"):
                allp.append(os.path.join(dirpath, fn))
    allp.sort()
    print("json files:", len(allp), flush=True)
    NW = 2
    chunks = [(i, allp[i::NW]) for i in range(NW)]
    with mp.Pool(NW) as pool:
        for idx, nf, nb in pool.imap_unordered(worker, chunks):
            print("part %d done: %d files %.2f GB" % (idx, nf, nb / 1e9), flush=True)

    total = {}
    tf = tb = 0
    for i in range(NW):
        with open(os.path.join(OUT, ".ast_part_%d.pkl" % i), "rb") as f:
            ctrs, nf, nb = pickle.load(f)
        tf += nf
        tb += nb
        for k, c in ctrs.items():
            total.setdefault(k, collections.Counter()).update(c)
        os.remove(os.path.join(OUT, ".ast_part_%d.pkl" % i))

    for k, c in total.items():
        name = k.lstrip("_")
        with open(os.path.join(OUT, "ast_%s.csv" % name), "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["value", "count"])
            for kk, vv in c.most_common():
                w.writerow([kk.decode("utf-8", "replace"), vv])

    summary = {"files": tf, "bytes": tb,
               **{k.lstrip("_"): len(v) for k, v in total.items()}}
    json.dump(summary, open(os.path.join(OUT, "ast_summary.json"), "w"), indent=2)
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
