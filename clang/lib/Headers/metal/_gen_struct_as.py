#!/usr/bin/env python3
"""Generate Metal struct address-space constructors and operators (cleanroom).

Usage: python3 _gen_struct_as.py <struct_name> [--methods <method_file>]
"""

import sys

# Address spaces for Metal structs
AS_DEFAULT = ["thread", "constant"]
AS_OPTIONAL = {
    "ray_data": "__HAVE_RAYTRACING__",
    "object_data": "__HAVE_MESH__",
    "device coherent(device)": "__HAVE_COHERENT__",
}
# Copy source AS (includes all)
AS_ALL = ["thread", "device", "device coherent(device)", "constant", "ray_data", "object_data"]

GUARDS = {
    "device coherent(device)": "__HAVE_COHERENT__",
    "ray_data": "__HAVE_RAYTRACING__",
    "object_data": "__HAVE_MESH__",
}

def needs_guard(as_name):
    return GUARDS.get(as_name)

def gen_guard_open(as_name):
    g = needs_guard(as_name)
    if g:
        return f"#if defined({g})\n  "
    return "  "

def gen_guard_close(as_name):
    g = needs_guard(as_name)
    if g:
        return "\n#endif"
    return ""

def gen_default_constructors(name):
    lines = []
    for as_name in AS_DEFAULT:
        lines.append(f"  METAL_FUNC {name}() {as_name} = default;")
    # Optional AS
    for as_name, guard in AS_OPTIONAL.items():
        if as_name == "device coherent(device)":
            continue  # coherent doesn't get default ctor in Apple's pattern
        lines.append(f"#if defined({guard})")
        lines.append(f"  METAL_FUNC {name}() {as_name} = default;")
        lines.append("#endif")
    return "\n".join(lines)

def gen_copy_constructors(name):
    lines = []
    # Dest: thread, constant, ray_data, object_data (NOT device or coherent)
    dest_list = ["thread", "constant", "ray_data", "object_data"]
    for dest in dest_list:
        dest_guard = needs_guard(dest)
        if dest_guard:
            lines.append(f"#if defined({dest_guard})")
        for src in AS_ALL:
            src_guard = needs_guard(src)
            # If both dest and src need guards, nest them
            if src_guard and dest_guard and src_guard != dest_guard:
                lines.append(f"#if defined({src_guard})")
                lines.append(f"  METAL_FUNC {name}(const {src} {name} &) {dest} = default;")
                lines.append("#endif")
            elif src_guard and not dest_guard:
                lines.append(f"#if defined({src_guard})")
                lines.append(f"  METAL_FUNC {name}(const {src} {name} &) {dest} = default;")
                lines.append("#endif")
            else:
                lines.append(f"  METAL_FUNC {name}(const {src} {name} &) {dest} = default;")
        if dest_guard:
            lines.append("#endif")
    return "\n".join(lines)

def gen_assignment_operators(name):
    lines = []
    # Dest: thread, device, device coherent(device), constant, ray_data, object_data
    dest_list = AS_ALL
    for dest in dest_list:
        dest_guard = needs_guard(dest)
        if dest_guard:
            lines.append(f"#if defined({dest_guard})")
        for src in AS_ALL:
            src_guard = needs_guard(src)
            if src_guard and dest_guard and src_guard != dest_guard:
                lines.append(f"#if defined({src_guard})")
                lines.append(f"  METAL_FUNC {dest} {name} &operator=(const {src} {name} &) {dest} = default;")
                lines.append("#endif")
            elif src_guard and not dest_guard:
                lines.append(f"#if defined({src_guard})")
                lines.append(f"  METAL_FUNC {dest} {name} &operator=(const {src} {name} &) {dest} = default;")
                lines.append("#endif")
            else:
                lines.append(f"  METAL_FUNC {dest} {name} &operator=(const {src} {name} &) {dest} = default;")
        if dest_guard:
            lines.append("#endif")
    return "\n".join(lines)

def gen_full_struct(name, extra_methods="", private_members=""):
    parts = []
    parts.append(f"struct {name}")
    parts.append("{")
    parts.append(gen_default_constructors(name))
    parts.append("")
    parts.append(gen_copy_constructors(name))
    parts.append("")
    parts.append(gen_assignment_operators(name))
    if extra_methods:
        parts.append("")
        parts.append(extra_methods)
    if private_members:
        parts.append("")
        parts.append("private:")
        parts.append(private_members)
    parts.append("};")
    return "\n".join(parts)

if __name__ == "__main__":
    name = sys.argv[1] if len(sys.argv) > 1 else "example_struct"
    print(gen_full_struct(name))
