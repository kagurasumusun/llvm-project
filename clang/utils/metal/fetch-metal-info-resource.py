#!/usr/bin/env python3
"""Fetch Apple Metal clang resource files from kagurasumusun/metal-info.

This intentionally does not clone metal-info. It uses GitHub's tree API plus
raw file downloads and writes a local clang resource-style directory containing
Apple Metal stdlib headers and Darwin Metal runtime libraries.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import urllib.parse
import urllib.request


def headers() -> dict[str, str]:
    h = {
        "Accept": "application/vnd.github+json",
        "X-GitHub-Api-Version": "2022-11-28",
        "User-Agent": "clang-metal-resource-fetch",
    }
    token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
    if token:
        h["Authorization"] = f"Bearer {token}"
    return h


def github_json(url: str):
    req = urllib.request.Request(url, headers=headers())
    with urllib.request.urlopen(req, timeout=90) as fh:
        return json.load(fh)


def download(url: str, out: Path) -> None:
    out.parent.mkdir(parents=True, exist_ok=True)
    req = urllib.request.Request(url, headers={"User-Agent": "clang-metal-resource-fetch"})
    token = os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
    if token:
        req.add_header("Authorization", f"Bearer {token}")
    with urllib.request.urlopen(req, timeout=120) as fh:
        out.write_bytes(fh.read())


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--repo", default="kagurasumusun/metal-info")
    ap.add_argument("--ref", default="main")
    ap.add_argument("--apple-clang-version", default="32023.883")
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument("--include-runtime", action="store_true")
    args = ap.parse_args()

    owner, repo = args.repo.split("/", 1)
    tree_url = f"https://api.github.com/repos/{owner}/{repo}/git/trees/{urllib.parse.quote(args.ref)}?recursive=1"
    tree = github_json(tree_url).get("tree", [])

    clang_root = f"reference-apple/clang/{args.apple_clang_version}"
    prefixes = [f"{clang_root}/include/metal/"]
    if args.include_runtime:
        prefixes.append(f"{clang_root}/lib/darwin/")

    selected = [
        row["path"]
        for row in tree
        if row.get("type") == "blob" and any(row.get("path", "").startswith(p) for p in prefixes)
    ]
    if not selected:
        raise SystemExit(f"no files found under {prefixes}")

    for path in sorted(selected):
        rel = path[len(clang_root) + 1 :]
        raw = f"https://raw.githubusercontent.com/{owner}/{repo}/{urllib.parse.quote(args.ref)}/{urllib.parse.quote(path, safe='/')}"
        download(raw, args.output / "clang" / args.apple_clang_version / rel)

    include_dir = args.output / "clang" / args.apple_clang_version / "include" / "metal"
    runtime_dir = args.output / "clang" / args.apple_clang_version / "lib" / "darwin"
    print(f"downloaded {len(selected)} file(s)")
    print(f"METAL_INFO_APPLE_CLANG_RESOURCE={args.output / 'clang' / args.apple_clang_version}")
    print(f"METAL_INFO_APPLE_METAL_INCLUDE={include_dir}")
    if args.include_runtime:
        print(f"METAL_INFO_APPLE_DARWIN_RUNTIME={runtime_dir}")


if __name__ == "__main__":
    main()
