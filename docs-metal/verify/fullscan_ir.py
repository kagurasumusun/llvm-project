#!/usr/bin/env python3
"""metal-info の全 .ll を 1 バイトも飛ばさずフルロード走査する。

出力: docs-metal/data/fullscan/ir_*.csv
全ファイル・全行を読み、以下を悉皆抽出する。
  - !air.* メタデータキー（文字列リテラル位置）と出現回数
  - !<name> = !{...} の名前付きメタデータ種別
  - air.* 呼び出し関数の完全シグネチャ（戻り型・引数型）
  - target triple / datalayout の全異形
  - addrspace(N) の全出現
  - function attribute の全キー
  - 型定義 %struct.* / opaque 型
  - calling convention
  - 命令ニーモニックの全出現
"""
import os, re, sys, collections, csv, json

ROOT = sys.argv[1] if len(sys.argv) > 1 else "/tmp/metal-info"
OUT = sys.argv[2] if len(sys.argv) > 2 else "/home/user/llvm-project/docs-metal/data/fullscan"
os.makedirs(OUT, exist_ok=True)

re_mdstr = re.compile(r'!"([^"]*)"')
re_named = re.compile(r'^!([A-Za-z_$][\w.$]*)\s*=\s*!\{')
re_triple = re.compile(r'^target triple = "([^"]*)"')
re_dl = re.compile(r'^target datalayout = "([^"]*)"')
re_as = re.compile(r'addrspace\((\d+)\)')
re_decl = re.compile(r'^declare[^@]*@([\w.$"\\]+)\(')
re_declfull = re.compile(r'^declare\s+(?:[^@]*?\s)?((?:[^@]|\s)*?)@([\w.$"\\]+)\((.*?)\)\s*(?:#\d+)?\s*$')
re_define = re.compile(r'^define\s+(.*?)@([\w.$"\\]+)\((.*)$')
re_call = re.compile(r'(?:call|invoke)[^@]*@([\w.$"\\]+)\(')
re_attrs = re.compile(r'^attributes #(\d+) = \{(.*)\}')
re_attrkv = re.compile(r'"([^"]+)"(?:="([^"]*)")?|([a-z_][\w-]*)(?:\((\d+)\))?')
re_typedef = re.compile(r'^(%[\w.":$\\ ]+?)\s*=\s*type\s+(.*)$')
re_instr = re.compile(r'^\s+(?:%[\w.]+\s*=\s*)?([a-z][\w.]*)\b')
re_cc = re.compile(r'^define\s+((?:\w+\s+)*)')
re_targetkw = re.compile(r'^target\s+(\w+)')

counters = {
    "mdstr": collections.Counter(),
    "named_md": collections.Counter(),
    "triple": collections.Counter(),
    "datalayout": collections.Counter(),
    "addrspace": collections.Counter(),
    "declared_fn": collections.Counter(),
    "called_fn": collections.Counter(),
    "defined_fn": collections.Counter(),
    "fn_attr": collections.Counter(),
    "typedef": collections.Counter(),
    "instr": collections.Counter(),
    "cc": collections.Counter(),
    "linkage": collections.Counter(),
    "global_kw": collections.Counter(),
    "misc_toplevel": collections.Counter(),
}
decl_sig = {}     # name -> set of "ret|args"
airmd_ctx = collections.Counter()  # "!air.x" first element context
files = 0
lines_total = 0
bytes_total = 0

CC_WORDS = {"ccc","fastcc","coldcc","webkit_jscc","anyregcc","preserve_mostcc",
            "preserve_allcc","cxx_fast_tlscc","swiftcc","swifttailcc","tailcc",
            "cfguard_checkcc","spir_func","spir_kernel","amdgpu_kernel",
            "ptx_kernel","ptx_device","intel_ocl_bicc","x86_stdcallcc"}
LINK_WORDS = {"private","internal","available_externally","linkonce","weak",
              "common","appending","extern_weak","linkonce_odr","weak_odr",
              "external","dllimport","dllexport","hidden","protected","default",
              "local_unnamed_addr","unnamed_addr","dso_local","dso_preemptable"}

for dirpath, dirnames, filenames in os.walk(ROOT):
    for fn in filenames:
        if not fn.endswith(".ll"):
            continue
        p = os.path.join(dirpath, fn)
        try:
            data = open(p, "r", errors="replace").read()
        except Exception:
            continue
        files += 1
        bytes_total += len(data)
        for line in data.split("\n"):
            lines_total += 1
            if not line:
                continue
            c0 = line[0]
            if c0 == "!":
                m = re_named.match(line)
                if m:
                    counters["named_md"][m.group(1)] += 1
                for s in re_mdstr.findall(line):
                    counters["mdstr"][s] += 1
                # 記録: !N = !{!"air.xxx", ...} の先頭文字列に続く全文字列列
                if line.startswith("!") and '= !{' in line:
                    strs = re_mdstr.findall(line)
                    if strs and strs[0].startswith("air."):
                        airmd_ctx["|".join(strs)] += 1
            elif c0 == "t":
                m = re_triple.match(line)
                if m:
                    counters["triple"][m.group(1)] += 1
                    continue
                m = re_dl.match(line)
                if m:
                    counters["datalayout"][m.group(1)] += 1
                    continue
                m = re_targetkw.match(line)
                if m:
                    counters["misc_toplevel"]["target " + m.group(1)] += 1
            elif c0 == "d":
                if line.startswith("declare"):
                    m = re_declfull.match(line.rstrip())
                    if m:
                        ret, name, args = m.group(1).strip(), m.group(2), m.group(3)
                        counters["declared_fn"][name] += 1
                        decl_sig.setdefault(name, set()).add(ret + " (" + args + ")")
                    else:
                        m2 = re_decl.match(line)
                        if m2:
                            counters["declared_fn"][m2.group(1)] += 1
                elif line.startswith("define"):
                    m = re_define.match(line)
                    if m:
                        pre, name, _ = m.group(1), m.group(2), m.group(3)
                        counters["defined_fn"][name] += 1
                        toks = pre.split()
                        cc = "ccc(default)"
                        for t in toks:
                            if t in CC_WORDS:
                                cc = t
                            elif t in LINK_WORDS:
                                counters["linkage"][t] += 1
                        counters["cc"][cc] += 1
            elif c0 == "a" and line.startswith("attributes #"):
                m = re_attrs.match(line)
                if m:
                    body = m.group(2)
                    for q, qv, b, bv in re_attrkv.findall(body):
                        if q:
                            counters["fn_attr"]['"%s"' % q] += 1
                        elif b:
                            counters["fn_attr"][b] += 1
            elif c0 == "%":
                m = re_typedef.match(line)
                if m:
                    counters["typedef"][m.group(1).strip() + " = type " + m.group(2).strip()] += 1
            elif c0 == "@":
                counters["global_kw"][line.split("=")[0].strip()[:80]] += 1
            elif c0 in " \t":
                m = re_instr.match(line)
                if m:
                    counters["instr"][m.group(1)] += 1
            for n in re_as.findall(line):
                counters["addrspace"][n] += 1
            for cn in re_call.findall(line):
                counters["called_fn"][cn] += 1

def dump(name, ctr):
    with open(os.path.join(OUT, "ir_%s.csv" % name), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["value", "count"])
        for k, v in ctr.most_common():
            w.writerow([k, v])

for k, c in counters.items():
    dump(k, c)
dump("airmd_pattern", airmd_ctx)
with open(os.path.join(OUT, "ir_decl_signatures.csv"), "w", newline="") as f:
    w = csv.writer(f)
    w.writerow(["function", "signature"])
    for k in sorted(decl_sig):
        for s in sorted(decl_sig[k]):
            w.writerow([k, s])

summary = {
    "files": files, "lines": lines_total, "bytes": bytes_total,
    **{k: len(v) for k, v in counters.items()},
    "airmd_pattern": len(airmd_ctx),
    "decl_signatures": sum(len(v) for v in decl_sig.values()),
}
json.dump(summary, open(os.path.join(OUT, "ir_summary.json"), "w"), indent=2)
print(json.dumps(summary, indent=2))
