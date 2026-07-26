#!/usr/bin/env python3
"""Generate Metal stdlib headers with address-space variants (cleanroom)."""

# Address spaces used by Apple Metal
# Base: thread, device, constant
# Optional: coherent(device), threadgroup, threadgroup_imageblock, ray_data, object_data

AS_BASE = ["thread", "device", "constant"]
AS_COHERENT = ["device coherent(device)"]
AS_THREADGROUP = ["threadgroup"]
AS_IMAGEBLOCK = ["threadgroup_imageblock"]
AS_RAYDATA = ["ray_data"]
AS_OBJECTDATA = ["object_data"]

# Copy sources for each destination
def gen_copy_constructors(name, dest_as_list, src_as_list, coherent_guard=False, raydata_guard=False, mesh_guard=False):
    lines = []
    for dest in dest_as_list:
        for src in src_as_list:
            guard_open = ""
            guard_close = ""
            if coherent_guard and "coherent" in dest:
                guard_open = "#if defined(__HAVE_COHERENT__)\n  "
                guard_close = "\n#endif"
            if raydata_guard and "ray_data" in dest:
                guard_open = "#if defined(__HAVE_RAYTRACING__)\n  "
                guard_close = "\n#endif"
            if mesh_guard and "object_data" in dest:
                guard_open = "#if defined(__HAVE_MESH__)\n  "
                guard_close = "\n#endif"
            if "coherent" in src:
                guard_open = "#if defined(__HAVE_COHERENT__)\n  "
                guard_close = "\n#endif"
            if "ray_data" in src:
                guard_open = "#if defined(__HAVE_RAYTRACING__)\n  "
                guard_close = "\n#endif"
            if "object_data" in src:
                guard_open = "#if defined(__HAVE_MESH__)\n  "
                guard_close = "\n#endif"
            lines.append(f"  {guard_open}METAL_FUNC {name}(const {src} {name} &) {dest} = default;{guard_close}")
    return "\n".join(lines)

def gen_assign_operators(name, dest_as_list, src_as_list):
    lines = []
    for dest in dest_as_list:
        for src in src_as_list:
            guard = ""
            if "coherent" in dest or "coherent" in src:
                guard = "#if defined(__HAVE_COHERENT__)\n  "
                guard_end = "\n#endif"
            elif "ray_data" in dest or "ray_data" in src:
                guard = "#if defined(__HAVE_RAYTRACING__)\n  "
                guard_end = "\n#endif"
            elif "object_data" in dest or "object_data" in src:
                guard = "#if defined(__HAVE_MESH__)\n  "
                guard_end = "\n#endif"
            else:
                guard_end = ""
            lines.append(f"  {guard}METAL_FUNC {dest} {name} &operator=(const {src} {name} &) {dest} = default;{guard_end}")
    return "\n".join(lines)

def gen_method_as(name, return_type, method_sig, body, as_list, const_method=False):
    """Generate a method with address-space variants."""
    lines = []
    for a in as_list:
        guard = ""
        guard_end = ""
        if "coherent" in a:
            guard = "#if defined(__HAVE_COHERENT__)\n  "
            guard_end = "\n#endif"
        elif "ray_data" in a:
            guard = "#if defined(__HAVE_RAYTRACING__)\n  "
            guard_end = "\n#endif"
        elif "object_data" in a:
            guard = "#if defined(__HAVE_MESH__)\n  "
            guard_end = "\n#endif"
        const_str = " const" if const_method else ""
        lines.append(f"  {guard}METAL_FUNC {return_type} {method_sig} {a}{const_str}\n  {{\n    {body}\n  }}{guard_end}")
    return "\n".join(lines)

if __name__ == "__main__":
    # Test: print array address-space accessors
    all_as = ["thread", "device", "device coherent(device)", "threadgroup", "threadgroup_imageblock", "ray_data"]
    print(gen_method_as("array", "constexpr size_t", "size()", "return N;", all_as, const_method=True))
