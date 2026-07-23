#!/usr/bin/env python3
"""Extract Metal frontend facts from Apple-clang AST dumps.

The input format is Clang's text AST dump as published by metal-info.  The
script accepts local paths or raw URLs and never needs to clone metal-info.  It
extracts two kinds of facts that are stable and useful for bootstrapping a
clean-room Metal frontend:

  * implicit __metal_* builtin object type names
  * Apple AST Metal*Attr spellings observed in the dumps

It can query the GitHub contents/tree APIs for one metal-info artifact directory
or all reference/metal-ast-* artifacts and select AST dumps from multiple Metal
language generations / fixtures.  This keeps the workflow compatible with the
"no full clone" constraint while still allowing broad batch refreshes.
"""

from __future__ import annotations

import argparse
import collections
import concurrent.futures
import json
import os
import pathlib
import re
import sys
import urllib.parse
import urllib.request

DEFAULT_FIXTURES = (
    "probe_min",
    "address_spaces_all",
    "attributes_all",
    "io_dump",
    "metal_object_types_all",
    "texture_read",
    "types_all",
    "stdlib_headers_all",
)

ATTR_ALIAS = {
    "MetalAttributeIndexAttr": "MetalAttributeAttr",
    "MetalBufferIndexAttr": "MetalBufferAttr",
    "MetalTextureIndexAttr": "MetalTextureAttr",
    "MetalSamplerIndexAttr": "MetalSamplerAttr",
    "MetalThreadPosGridAttr": "MetalThreadPositionInGridAttr",
    "MetalThreadPosGroupAttr": "MetalThreadPositionInThreadgroupAttr",
    "MetalThreadIndexGroupAttr": "MetalThreadIndexInThreadgroupAttr",
    "MetalThreadsPerGroupAttr": "MetalThreadsPerThreadgroupAttr",
    "MetalUserDefinedAttr": "MetalUserAttr",
    "MetalKernelAttr": "DeviceKernelAttr",
}

BUILTIN_RE = re.compile(r"implicit (__metal_[A-Za-z0-9_]+) '(__metal_[A-Za-z0-9_]+)'")
ATTR_RE = re.compile(r"\b(Metal[A-Za-z0-9_]*Attr)\b")
STD_RE = re.compile(r"_(?:macos|osx|ios|tvos|watchos)?-?metal(?P<std>[0-9]+\.[0-9]+)_")
ARTIFACT_RE = re.compile(r"reference/(?P<artifact>metal-ast-[^/]+)/ast/")


def read_text(path_or_url: str) -> str:
    if path_or_url.startswith(("http://", "https://")):
        req = urllib.request.Request(path_or_url, headers={"User-Agent": "clang-metal-ast-table-gen"})
        with urllib.request.urlopen(req, timeout=60) as fh:
            return fh.read().decode("utf-8", errors="replace")
    return pathlib.Path(path_or_url).read_text(encoding="utf-8", errors="replace")


def github_json(url: str):
    headers = {
        "Accept": "application/vnd.github+json",
        "User-Agent": "clang-metal-ast-table-gen",
    }
    token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
    if token:
        headers["Authorization"] = f"Bearer {token}"
    req = urllib.request.Request(url, headers=headers)
    with urllib.request.urlopen(req, timeout=60) as fh:
        return json.load(fh)


def discover_reference_artifacts(repo: str, ref: str) -> list[str]:
    owner, name = repo.split("/", 1)
    url = f"https://api.github.com/repos/{owner}/{name}/contents/reference?ref={urllib.parse.quote(ref)}"
    rows = github_json(url)
    return sorted(
        row["name"]
        for row in rows
        if row.get("type") == "dir" and row.get("name", "").startswith("metal-ast-")
    )


def discover_metal_info_inputs(repo: str, ref: str, artifact: str,
                               fixtures: tuple[str, ...]) -> list[str]:
    """Discover ast-text raw URLs for a single reference artifact.

    Uses the git tree API for the artifact's ast/ directory instead of the
    contents API, because large ast/ directories exceed the contents endpoint's
    1000-entry limit.
    """
    owner, name = repo.split("/", 1)
    artifact_api_path = urllib.parse.quote(f"reference/{artifact}", safe="/")
    contents_url = (
        f"https://api.github.com/repos/{owner}/{name}/contents/"
        f"{artifact_api_path}?ref={urllib.parse.quote(ref)}"
    )
    try:
        artifact_rows = github_json(contents_url)
    except Exception as exc:
        print(f"warning: failed to list artifact {artifact}: {exc}", file=sys.stderr)
        return []

    ast_sha = None
    for row in artifact_rows:
        if row.get("name") == "ast" and row.get("type") == "dir":
            ast_sha = row.get("sha")
            break
    if not ast_sha:
        return []

    tree_url = f"https://api.github.com/repos/{owner}/{name}/git/trees/{ast_sha}?recursive=1"
    tree = github_json(tree_url)
    if tree.get("truncated"):
        print(f"warning: ast tree for {artifact} is truncated", file=sys.stderr)

    inputs: list[str] = []
    for row in tree.get("tree", []):
        filename = row.get("path", "")
        if row.get("type") != "blob" or not filename.endswith("_ast-text.txt"):
            continue
        if fixtures and not any(f"_{fixture}_" in filename for fixture in fixtures):
            continue
        raw_path = urllib.parse.quote(f"reference/{artifact}/ast/{filename}", safe="/")
        inputs.append(
            f"https://raw.githubusercontent.com/{owner}/{name}/{urllib.parse.quote(ref)}/{raw_path}"
        )
    return sorted(inputs)


def extract_one(item: str):
    text = read_text(item)
    attrs = collections.Counter(ATTR_RE.findall(text))
    builtins = collections.Counter(
        builtin for builtin, spelling in BUILTIN_RE.findall(text) if builtin == spelling
    )
    std = None
    artifact = None
    name = pathlib.PurePosixPath(urllib.parse.urlparse(item).path).name
    if m := STD_RE.search(name):
        std = m.group("std")
    if m := ARTIFACT_RE.search(item):
        artifact = m.group("artifact")
    return item, attrs, builtins, std, artifact


def extract(inputs: list[str], jobs: int):
    attrs = collections.Counter()
    builtins = collections.Counter()
    standards = collections.Counter()
    artifacts = collections.Counter()
    used_inputs: list[str] = []

    def consume(result):
        item, a, b, std, artifact = result
        used_inputs.append(item)
        attrs.update(a)
        builtins.update(b)
        if std:
            standards[std] += 1
        if artifact:
            artifacts[artifact] += 1

    if jobs <= 1:
        for item in inputs:
            try:
                consume(extract_one(item))
            except Exception as exc:
                print(f"warning: failed to read {item}: {exc}", file=sys.stderr)
    else:
        with concurrent.futures.ThreadPoolExecutor(max_workers=jobs) as executor:
            future_to_item = {executor.submit(extract_one, item): item for item in inputs}
            for future in concurrent.futures.as_completed(future_to_item):
                item = future_to_item[future]
                try:
                    consume(future.result())
                except Exception as exc:
                    print(f"warning: failed to read {item}: {exc}", file=sys.stderr)

    used_inputs.sort()
    return used_inputs, attrs, builtins, standards, artifacts


def std_key(std: str):
    return tuple(int(part) for part in std.split("."))


def emit_def(inputs: list[str], attrs, builtins, standards, artifacts) -> str:
    out: list[str] = []
    out.append("//===--- MetalASTReference.def - Facts from Metal AST dumps ----*- C++ -*-===//")
    out.append("//")
    out.append("// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.")
    out.append("// See https://llvm.org/LICENSE.txt for license information.")
    out.append("// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception")
    out.append("//")
    out.append("//===----------------------------------------------------------------------===//")
    out.append("//")
    out.append("// Generated by clang/utils/metal/gen-metal-ast-support.py from Apple-clang")
    out.append("// AST dumps, typically from kagurasumusun/metal-info reference artifacts.")
    out.append("//")
    if standards:
        out.append("// Metal standards represented in scanned AST dumps:")
        for std, count in sorted(standards.items(), key=lambda kv: std_key(kv[0])):
            out.append(f"//   metal{std}: {count} dump(s)")
        out.append("//")
    if artifacts:
        out.append("// Reference artifacts represented:")
        for artifact, count in sorted(artifacts.items()):
            out.append(f"//   {artifact}: {count} dump(s)")
        out.append("//")
    out.append(f"// Input AST dumps scanned: {len(inputs)}")
    out.append("//")
    out.append("// METAL_AST_BUILTIN_TYPE(__metal_foo_t)")
    out.append("// METAL_AST_ATTR(AppleASTAttrName)")
    out.append("// METAL_AST_ATTR_ALIAS(AppleASTAttrName, CurrentClangAttrName)")
    out.append("//")
    out.append("//===----------------------------------------------------------------------===//")
    out.append("")
    out.append("#ifndef METAL_AST_BUILTIN_TYPE")
    out.append("#define METAL_AST_BUILTIN_TYPE(Name)")
    out.append("#endif")
    out.append("#ifndef METAL_AST_ATTR")
    out.append("#define METAL_AST_ATTR(Name)")
    out.append("#endif")
    out.append("#ifndef METAL_AST_ATTR_ALIAS")
    out.append("#define METAL_AST_ATTR_ALIAS(AppleName, ClangName)")
    out.append("#endif")
    out.append("")
    for name, count in sorted(builtins.items()):
        out.append(f"// observed {count} time(s)")
        out.append(f"METAL_AST_BUILTIN_TYPE({name})")
    out.append("")
    for name, count in sorted(attrs.items()):
        out.append(f"// observed {count} time(s)")
        out.append(f"METAL_AST_ATTR({name})")
        if name in ATTR_ALIAS:
            out.append(f"METAL_AST_ATTR_ALIAS({name}, {ATTR_ALIAS[name]})")
    out.append("")
    return "\n".join(out)


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True)
    parser.add_argument("--input", action="append", default=[], help="Local AST dump path or raw URL")
    parser.add_argument("--metal-info-repo", default="kagurasumusun/metal-info")
    parser.add_argument("--metal-info-ref", default="main")
    parser.add_argument("--artifact", action="append", default=[], help="metal-info reference artifact to scan")
    parser.add_argument("--all-artifacts", action="store_true", help="Scan every reference/metal-ast-* artifact with an ast/ tree")
    parser.add_argument("--fixture", action="append", default=[], help="Fixture basename to select when discovering metal-info AST dumps")
    parser.add_argument("--all-fixtures", action="store_true", help="Do not filter discovered AST dumps by fixture name")
    parser.add_argument("--discover-metal-info", action="store_true", help="Discover AST dumps from the metal-info GitHub API")
    parser.add_argument("--jobs", type=int, default=8, help="Concurrent raw AST fetches")
    parser.add_argument("--max-inputs", type=int, default=0, help="Debugging limit after discovery")
    args = parser.parse_args(argv)

    inputs = list(args.input)
    if args.discover_metal_info or args.all_artifacts or args.artifact:
        artifacts = list(args.artifact)
        if args.all_artifacts:
            artifacts.extend(discover_reference_artifacts(args.metal_info_repo, args.metal_info_ref))
        if not artifacts:
            artifacts.append("metal-ast-macos-air64")
        fixtures = () if args.all_fixtures else tuple(args.fixture or DEFAULT_FIXTURES)
        for artifact in sorted(set(artifacts)):
            inputs.extend(discover_metal_info_inputs(args.metal_info_repo, args.metal_info_ref,
                                                     artifact, fixtures))
    if args.max_inputs:
        inputs = inputs[:args.max_inputs]
    # Preserve deterministic processing and avoid duplicate URLs.
    inputs = sorted(dict.fromkeys(inputs))
    if not inputs:
        parser.error("provide --input or enable discovery with --discover-metal-info/--all-artifacts/--artifact")

    used_inputs, attrs, builtins, standards, artifacts = extract(inputs, max(1, args.jobs))
    output = pathlib.Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(emit_def(used_inputs, attrs, builtins, standards, artifacts), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
