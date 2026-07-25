#!/usr/bin/env python3
"""Build a deterministic Metal compatibility manifest from observed Apple data.

This intentionally records observations rather than inventing declarations.
Apple AST JSON supplies canonical function and parameter types; the header tree
supplies feature macro and address-space vocabulary.  Downstream TableGen/Sema
and CodeGen generators consume this JSON as a reviewable source of truth.
"""
from __future__ import annotations
import argparse, json, re
from collections import defaultdict
from pathlib import Path

BUILTIN = re.compile(r"^__metal_[A-Za-z0-9_]+$")
ADDR = re.compile(r"\b(thread|device|constant|threadgroup|threadgroup_imageblock|ray_data|object_data)\b")
FEATURE = re.compile(r"^\s*#\s*define\s+(__HAVE_[A-Z0-9_]+__|__METAL_[A-Z0-9_]+__)\b(?:\s+(.*))?$")

def walk(node):
    if isinstance(node, dict):
        yield node
        for child in node.get("inner", []):
            yield from walk(child)

def typ(node):
    return node.get("type", {}).get("qualType", "")

def function_rows(doc, origin):
    rows = []
    for node in walk(doc):
        if node.get("kind") != "FunctionDecl" or not BUILTIN.match(node.get("name", "")):
            continue
        params = [typ(x) for x in node.get("inner", []) if x.get("kind") == "ParmVarDecl"]
        row = {"name": node["name"], "type": typ(node), "parameters": params, "origin": origin}
        rows.append(row)
    return rows

def header_data(root):
    features, address_spaces, names = {}, set(), set()
    for path in sorted(root.rglob("*")):
        if not path.is_file() or path.name == "module.modulemap":
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        names.update(BUILTIN.findall(text))
        address_spaces.update(ADDR.findall(text))
        for match in FEATURE.finditer(text):
            features.setdefault(match.group(1), match.group(2) or "1")
    return features, sorted(address_spaces), sorted(names)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ast", action="append", type=Path, default=[])
    ap.add_argument("--headers", type=Path, required=True)
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument("--apple-clang-version", default="unknown")
    args = ap.parse_args()
    features, spaces, header_names = header_data(args.headers)
    seen, overloads = set(), defaultdict(list)
    for ast in args.ast:
        doc = json.loads(ast.read_text(encoding="utf-8"))
        for row in function_rows(doc, str(ast)):
            key = (row["name"], row["type"], tuple(row["parameters"]))
            if key not in seen:
                seen.add(key); overloads[row["name"]].append(row)
    result = {
        "schema": 1,
        "apple_clang_version": args.apple_clang_version,
        "observed_address_spaces": spaces,
        "feature_macros": dict(sorted(features.items())),
        "header_builtin_names": header_names,
        "observed_builtin_overloads": {k: sorted(v, key=lambda x: (x["type"], x["parameters"])) for k, v in sorted(overloads.items())},
        "unobserved_header_builtin_names": sorted(set(header_names) - set(overloads)),
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
if __name__ == "__main__": main()
