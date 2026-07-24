#!/usr/bin/env python3
"""Fetch Apple Metal stdlib/resource files from metal-info without cloning.

The script downloads selected paths from kagurasumusun/metal-info through the
GitHub tree/raw APIs.  It is intended for CI smoke tests that need Apple's real
Metal headers and runtime artifacts temporarily in the workspace.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import sys
import urllib.parse
import urllib.request


def request_headers() -> dict[str, str]:
    headers = {
        "Accept": "application/vnd.github+json",
        "User-Agent": "clang-metal-info-resource-fetch",
    }
    token = os.environ.get("METAL_INFO_GITHUB_TOKEN") or os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")
    if token:
        headers["Authorization"] = f"Bearer {token}"
    return headers


def github_json(url: str):
    req = urllib.request.Request(url, headers=request_headers())
    with urllib.request.urlopen(req, timeout=120) as fh:
        return json.load(fh)


def raw_bytes(repo: str, ref: str, path: str) -> bytes:
    owner, name = repo.split("/", 1)
    url = f"https://raw.githubusercontent.com/{owner}/{name}/{urllib.parse.quote(ref, safe='')}/{urllib.parse.quote(path, safe='/')}"
    req = urllib.request.Request(url, headers={"User-Agent": "clang-metal-info-resource-fetch"})
    with urllib.request.urlopen(req, timeout=120) as fh:
        return fh.read()


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--repo", default="kagurasumusun/metal-info")
    parser.add_argument("--ref", default="main")
    parser.add_argument("--apple-clang-version", default="32023.883")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--include-metal", action="store_true")
    parser.add_argument("--lib-darwin", action="store_true")
    args = parser.parse_args(argv)

    prefixes: list[str] = []
    root = f"reference-apple/clang/{args.apple_clang_version}"
    if args.include_metal:
        prefixes.append(f"{root}/include/metal/")
    if args.lib_darwin:
        prefixes.append(f"{root}/lib/darwin/")
    if not prefixes:
        parser.error("select at least one of --include-metal or --lib-darwin")

    owner, name = args.repo.split("/", 1)
    tree_url = f"https://api.github.com/repos/{owner}/{name}/git/trees/{urllib.parse.quote(args.ref, safe='')}?recursive=1"
    tree = github_json(tree_url).get("tree", [])
    files = [row["path"] for row in tree if row.get("type") == "blob" and any(row.get("path", "").startswith(prefix) for prefix in prefixes)]
    files.sort()
    if not files:
        raise SystemExit("no files matched requested metal-info resource prefixes")

    for index, path in enumerate(files, 1):
        out = args.output / path
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_bytes(raw_bytes(args.repo, args.ref, path))
        print(f"[{index}/{len(files)}] {path}")

    print(f"Fetched {len(files)} file(s) into {args.output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
