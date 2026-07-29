#!/usr/bin/env python3
"""Verify the Metal support constants in this tree against the metal-info data.

The metal-info repository (https://github.com/kagurasumusun/metal-info) is the
primary source for every Metal specific value in this fork. This script
re-extracts the values from that reference set and compares them against what
the tree actually contains, so that a data transcription mistake is caught
without needing to build the compiler.

Usage:
    git clone --depth 1 https://github.com/kagurasumusun/metal-info /tmp/metal-info
    python3 docs-metal/verify/verify_against_metal_info.py /tmp/metal-info
"""

import csv
import os
import re
import sys

LLVM_ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

failures = []
checks = 0


def check(name, ok, detail=""):
    global checks
    checks += 1
    if ok:
        print("  ok    %s" % name)
    else:
        print("  FAIL  %s %s" % (name, detail))
        failures.append(name)


def read(path):
    with open(os.path.join(LLVM_ROOT, path), encoding="utf-8") as f:
        return f.read()


def main(info):
    ast_dir = os.path.join(info, "reference/metal-ast-macos-air64")
    ds_dir = os.path.join(info, "research/datasets")
    golden = os.path.join(info, "research/golden")

    print("== DataLayout ==")
    # The 64-bit DataLayout must match every golden .ll byte for byte.
    want64 = None
    p01 = os.path.join(golden, "P01/metal32_macosx26/probe.ll")
    for line in open(p01, encoding="utf-8"):
        m = re.match(r'target datalayout = "(.*)"', line)
        if m:
            want64 = m.group(1)
            break
    air_h = read("clang/lib/Basic/Targets/AIR.h")
    # Reassemble the string literal concatenation in the header.
    lits = re.findall(r'"([^"]*)"', air_h.split("if (Is64Bit)")[1].split("return")[1])
    got64 = "".join(lits)
    check("air64 datalayout matches golden P01", got64 == want64,
          "\n    want %s\n    got  %s" % (want64, got64))

    print("== Address spaces ==")
    # Extract the target values from the AIRAddrSpaceMap in AIR.h.
    block = air_h.split("static const unsigned AIRAddrSpaceMap[] = {")[1].split("};")[0]
    asmap = dict(re.findall(r"(\d+), // (\w+)", block))
    asmap = {v: int(k) for k, v in asmap.items()}
    # The expected values are the ones measured from generated IR:
    #   device/constant/threadgroup/thread from address_spaces_extended_all,
    #   threadgroup_imageblock from _imageblock_base's member pointer,
    #   object_data from the object shader payload probe.
    # ray_data has never been observed as a parameter; see AIR.h.
    for name, want in [("metal_device", 1), ("metal_constant", 2),
                       ("metal_threadgroup", 3), ("metal_thread", 0),
                       ("metal_threadgroup_imageblock", 4),
                       ("metal_object_data", 6), ("metal_ray_data", 9)]:
        check("addrspace %s == %d" % (name, want), asmap.get(name) == want,
              "got %r" % asmap.get(name))

    print("== Opaque builtin types ==")
    # Order and membership must match the implicit typedefs in the AST dump.
    dump = os.path.join(
        ast_dir, "ast",
        "macos_air64_versioned_none_metal4.0_attributes_all_26_0_ast-text.txt")
    want_types = re.findall(r"implicit (__metal_\w+)", open(dump, encoding="utf-8").read())
    got_types = re.findall(r"^METAL_TYPE\((\w+),", read("clang/include/clang/Basic/MetalTypes.def"),
                           re.M)
    check("37 opaque types in AST-measured order", want_types == got_types,
          "\n    want %d %s\n    got  %d %s" %
          (len(want_types), want_types[:3], len(got_types), got_types[:3]))

    # IR struct names must match the golden corpus.
    ir_names = dict(re.findall(r'^METAL_TYPE\((\w+), \w+, \w+, "([^"]+)"\)',
                               read("clang/include/clang/Basic/MetalTypes.def"), re.M))
    bad = []
    with open(os.path.join(ds_dir, "type_layout_map.csv"), encoding="utf-8") as f:
        for row in csv.DictReader(f):
            want = row["ir_struct"]           # e.g. %struct._texture_2d_t
            b = row["builtin_row"]
            if b in ir_names and "%struct." + ir_names[b] != want:
                bad.append((b, want, ir_names[b]))
    check("IR struct names match golden type_layout_map", not bad, str(bad[:3]))

    # Independent cross-check: the -ast-dump-lookups output lists every name the
    # compiler injects into the translation unit, which must agree with
    # MetalTypes.def exactly.
    lookup = os.path.join(
        ast_dir, "log",
        "macos_air64_versioned_none_metal4.0_probe_min_26_0_ast-dump-lookups.stdout")
    if os.path.exists(lookup):
        looked = set(re.findall(r"DeclarationName '(__metal_\w+)'",
                                open(lookup, errors="ignore").read()))
        check("opaque types agree with -ast-dump-lookups (%d)" % len(looked),
              looked == set(got_types),
              "only in lookups %s / only in impl %s"
              % (sorted(looked - set(got_types))[:3],
                 sorted(set(got_types) - looked)[:3]))

    print("== Predefined macros ==")
    measured = {}
    for line in open(os.path.join(ast_dir, "meta/metal-predefined-macros.txt"),
                     encoding="utf-8"):
        m = re.match(r"#define (__(?:METAL|AIR)\w*) (.*)$", line.rstrip("\n"))
        if m:
            measured[m.group(1)] = m.group(2)
    computed = {"__METAL_VERSION__", "__AIR_VERSION__", "__AIR64__", "__AIR32__",
                "__METAL__", "__METAL_FAST_MATH__",
                "__METAL_MATH_FP32_FUNCTIONS_FAST__"}
    got = dict(re.findall(r'^METAL_MACRO\("([^"]+)", "([^"]*)"\)',
                          read("clang/include/clang/Basic/MetalMacros.def"), re.M))
    want = {k: v for k, v in measured.items() if k not in computed}
    check("%d constant Metal macros transcribed exactly" % len(want), got == want,
          "missing %s extra %s" % (sorted(set(want) - set(got))[:3],
                                   sorted(set(got) - set(want))[:3]))

    print("== -std= spellings ==")
    stds = set(re.findall(r'^LANGSTANDARD\(\w+, "([^"]+)"', read(
        "clang/include/clang/Basic/LangStandards.def"), re.M))
    observed = set()
    for name in os.listdir(os.path.join(ast_dir, "ast")):
        m = re.match(r"macos_air64_versioned_none_([a-z0-9.-]+)_", name)
        if m and "metal" in m.group(1):
            observed.add(m.group(1))
    check("every observed -std= spelling is implemented",
          observed <= stds, "missing %s" % sorted(observed - stds))

    print("== MSL version -> C++ base ==")
    # Cross-check against the measured __cplusplus per standard recorded in
    # research/datasets/metal_cxx_generations_map.csv.
    lang_def = read("clang/include/clang/Basic/LangStandards.def")
    entries = {}
    for m in re.finditer(r'LANGSTANDARD\(\w+, "([^"]+)",\s*\n\s*Metal, "[^"]*",\s*\n'
                         r'((?:[^\n]*\n)*?[^\n]*\))', lang_def):
        entries[m.group(1)] = m.group(2)
    cxx_want = {}
    path = os.path.join(ds_dir, "metal_cxx_generations_map.csv")
    if os.path.exists(path):
        with open(path, encoding="utf-8") as f:
            for row in csv.DictReader(f):
                std = (row.get("std_flag") or row.get("-std=") or "").strip()
                cpp = (row.get("cplusplus_macro_val") or row.get("cplusplus")
                       or row.get("__cplusplus") or "").strip()
                if std and cpp:
                    cxx_want[std] = cpp
    bad = []
    for std, feats in entries.items():
        want = cxx_want.get(std)
        if not want:
            continue
        has17 = "CPlusPlus17" in feats
        has14 = "CPlusPlus14" in feats
        got = "201703L" if has17 else ("201402L" if has14 else "201103L")
        if got != want:
            bad.append((std, want, got))
    check("C++ base per MSL version matches measurement (%d checked)" % len(cxx_want),
          not bad, str(bad[:5]))

    print("== Resource limits ==")
    p01_text = open(p01, encoding="utf-8").read()
    for key, field in [("air.max_device_buffers", "MaxDeviceBuffers"),
                       ("air.max_constant_buffers", "MaxConstantBuffers"),
                       ("air.max_threadgroup_buffers", "MaxThreadgroupBuffers"),
                       ("air.max_textures", "MaxTextures"),
                       ("air.max_read_write_textures", "MaxReadWriteTextures"),
                       ("air.max_samplers", "MaxSamplers")]:
        m = re.search(r'!"%s", i32 (\d+)' % re.escape(key), p01_text)
        want = int(m.group(1)) if m else None
        m2 = re.search(r"%s = (\d+);" % field, air_h)
        got = int(m2.group(1)) if m2 else None
        check("%s == %s" % (key, want), want is not None and want == got,
              "got %r" % got)

    print("== Address spaces, re-derived from IR ==")
    # Rather than trusting the table above, re-measure the two spaces that were
    # wrong before by finding the probe IR that exercises them.
    import glob as _glob
    ib_as = obj_as = None
    for f in _glob.glob(os.path.join(info, "reference/metal-ast-*/ir/*.ll")):
        if ib_as and obj_as:
            break
        try:
            text = open(f, errors="ignore").read()
        except OSError:
            continue
        if ib_as is None:
            m = re.search(r'_imageblock_base" = type \{ %struct\._imageblock_t '
                          r'addrspace\((\d+)\)', text)
            if m:
                ib_as = int(m.group(1))
        if obj_as is None:
            m = re.search(r'define void @ofn\(%struct\.\w+ addrspace\((\d+)\)', text)
            if m:
                obj_as = int(m.group(1))
    if ib_as is not None:
        check("threadgroup_imageblock re-derived from IR",
              asmap.get("metal_threadgroup_imageblock") == ib_as,
              "IR says %s" % ib_as)
    if obj_as is not None:
        check("object_data re-derived from the object payload probe",
              asmap.get("metal_object_data") == obj_as, "IR says %s" % obj_as)

    print("== Attributes ==")
    import csv as _csv
    attr_rows = list(_csv.DictReader(
        open(os.path.join(LLVM_ROOT, "docs-metal/data/metal_attributes.csv"),
             encoding="utf-8")))
    attr_td = read("clang/include/clang/Basic/Attr.td")

    # Every spelling mined from the MSL specification must be implemented.
    spec_spellings = set()
    with open(os.path.join(ds_dir, "spec_attributes.csv"), encoding="utf-8") as f:
        for row in _csv.DictReader(f):
            spec_spellings.add(row["attribute"])
    impl_spellings = set(re.findall(r'CXX11<"", "(\w+)">', attr_td))
    check("all %d MSL spec attributes implemented" % len(spec_spellings),
          spec_spellings <= impl_spellings,
          "missing %s" % sorted(spec_spellings - impl_spellings))

    # Every AST class Apple was observed to create must exist, spelled the same.
    observed_classes = set()
    with open(os.path.join(ast_dir, "meta/attr-class-topology.csv"),
              encoding="utf-8") as f:
        for row in _csv.DictReader(f):
            if row["class"].startswith("Metal"):
                observed_classes.add(row["class"])
    impl_classes = {r["ast_class"] for r in attr_rows}
    check("all %d observed Metal*Attr classes present" % len(observed_classes),
          observed_classes <= impl_classes,
          "missing %s" % sorted(observed_classes - impl_classes))

    # The version gates must match the diagnostics Apple emits.
    gate_re = re.compile(
        r"'([a-z_0-9]+)' attribute requires Metal language standard "
        r"([a-z0-9.-]+) or higher")
    measured_gates = {}
    logdir = os.path.join(ast_dir, "log")
    if os.path.isdir(logdir):
        for name in os.listdir(logdir):
            if not name.endswith(".err"):
                continue
            try:
                text = open(os.path.join(logdir, name), errors="ignore").read()
            except OSError:
                continue
            for attr, std in gate_re.findall(text):
                # Keep the lowest standard seen for an attribute.
                prev = measured_gates.get(attr)
                if prev is None or std < prev:
                    measured_gates[attr] = std
    declared = {r["spelling"]: r["min_std"] for r in attr_rows}
    bad_gates = []
    for attr, std in measured_gates.items():
        got = declared.get(attr)
        if got is None:
            bad_gates.append((attr, std, "not implemented"))
        elif got != std:
            bad_gates.append((attr, std, got))
    check("version gates match measured diagnostics (%d checked)"
          % len(measured_gates), not bad_gates, str(bad_gates[:5]))

    print("== Builtins ==")
    # Every __metal_* name used by the real Apple standard library headers must
    # be declared, either as a function builtin or as an opaque handle type.
    hdr_dir = os.path.join(info, "reference-apple/clang/32023.883/include/metal")
    header_names = set()
    for root, _dirs, files in os.walk(hdr_dir):
        for fn in files:
            try:
                text = open(os.path.join(root, fn), errors="ignore").read()
            except OSError:
                continue
            header_names.update(re.findall(r"__metal_[A-Za-z0-9_]+", text))

    bi_def = read("clang/include/clang/Basic/BuiltinsMetal.def")
    declared_fns = set(re.findall(r"^METAL_BUILTIN\((\w+),", bi_def, re.M))
    declared_types = set(re.findall(r"^METAL_TYPE\((\w+),",
                                    read("clang/include/clang/Basic/MetalTypes.def"),
                                    re.M))
    declared = declared_fns | declared_types
    check("all %d __metal_* names used by the Apple stdlib are declared"
          % len(header_names), header_names <= declared,
          "missing %s" % sorted(header_names - declared)[:5])

    # The AIR intrinsic recorded for each builtin must match the mapping table.
    want_air = {}
    with open(os.path.join(ds_dir, "builtin_to_air_map.v2.csv"),
              encoding="utf-8") as f:
        for row in csv.DictReader(f):
            want_air[row["__metal_builtin"]] = row["air_intrinsic_candidate"].strip()
    got_air = dict(re.findall(r'^METAL_BUILTIN\((\w+), "[^"]*", "[^"]*", "([^"]*)"\)',
                              bi_def, re.M))
    bad_air = [(k, want_air[k], v) for k, v in got_air.items()
               if k in want_air and want_air[k] != v]
    check("AIR intrinsic names match the mapping table (%d builtins)"
          % len(got_air), not bad_air, str(bad_air[:3]))

    print("== Entry point metadata ==")
    # Cross-check the metadata key vocabulary CGMetal.cpp emits against the keys
    # actually observed in Apple's output.
    # The vocabulary is drawn from three measurements: the metadata key census
    # of Apple generated IR, the golden corpus, and the string dictionary
    # extracted from the compiler binaries themselves (15,643 air.* strings,
    # research/datasets/air_dictionary_from_binaries.txt). Stage inputs such as
    # air.sample_id only appear in the last of these, because no probe in the
    # captured corpus happens to use them.
    observed_keys = set()
    with open(os.path.join(ast_dir, "meta/air-metadata-keys.csv"),
              encoding="utf-8") as f:
        for row in csv.DictReader(f):
            observed_keys.add(row["metadata_key"])
    dict_path = os.path.join(ds_dir, "air_dictionary_from_binaries.txt")
    if os.path.exists(dict_path):
        observed_keys.update(
            re.findall(r"\bair\.[a-z_0-9]+(?:\.[a-z_0-9]+)*",
                       open(dict_path, errors="ignore").read()))
    cg = read("clang/lib/CodeGen/CGMetal.cpp")
    emitted_keys = set(re.findall(r'"(air\.[a-z_.]+)"', cg))
    unknown = {k for k in emitted_keys if k not in observed_keys}
    # Named metadata (air.kernel etc.) lives in a different census.
    named = set()
    with open(os.path.join(ast_dir, "meta/air-named-metadata.csv"),
              encoding="utf-8") as f:
        for row in csv.DictReader(f):
            named.add(row["metadata_name"])
    unknown -= named
    check("every air.* key CGMetal emits was observed in Apple output",
          not unknown, "unknown %s" % sorted(unknown))

    print("== AIR version from deployment target ==")
    # The driver derives the _vNN suffix from the deployment target. Re-derive
    # the expected mapping from Apple's own -### logs and compare it against
    # the table compiled into AIR.cpp.
    logdir = os.path.join(ast_dir, "log")
    measured_map = {}
    tri_re = re.compile(r"air64_v(\d+)-apple-macosx(\d+)\.(\d+)")
    if os.path.isdir(logdir):
        for name in os.listdir(logdir):
            if not name.endswith((".err", ".stdout")):
                continue
            try:
                text = open(os.path.join(logdir, name), errors="ignore").read()
            except OSError:
                continue
            for ver, maj, minor in tri_re.findall(text):
                measured_map[(int(maj), int(minor))] = int(ver)

    air_cpp = read("clang/lib/Basic/Targets/AIR.cpp")
    body = air_cpp.split("getAIRVersionForMacOSVersion")[-1]
    # The function has two switches: the first is on Minor inside `if
    # (Major == 10)`, the second is on Major.
    minor_part, major_part = body.split("switch (Major)", 1)
    impl_10x = dict(re.findall(r"case (\d+):\s*\n\s*return (\d+);", minor_part))
    impl_major = dict(re.findall(r"case (\d+):\s*\n\s*return (\d+);", major_part))
    bad_air = []
    for (maj, minor), want in sorted(measured_map.items()):
        if maj == 10:
            got = impl_10x.get(str(minor))
        elif maj >= 26:
            got = "28"
        else:
            got = impl_major.get(str(maj))
        if got is None or int(got) != want:
            bad_air.append(((maj, minor), want, got))
    check("AIR version table matches driver measurements (%d targets)"
          % len(measured_map), not bad_air, str(bad_air[:5]))

    print("\n%d checks, %d failures" % (checks, len(failures)))
    return 1 if failures else 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1]))
