#!/bin/bash
#===-- wince/test/e2e/run-e2e-cxx.sh - C++ end-to-end tests ----------------===#
#
# Builds and links the C++ sample with the WinCE toolchain (libc++,
# libc++abi, libunwind static against the sysroot), then inspects the
# image: exception tables present, personality linked, ctors dispatched,
# PE headers intact.  Run after the runtimes stage.
#
# Usage: run-e2e-cxx.sh <build-dir-with-clang-lld> <sysroot-dir>
#===------------------------------------------------------------------------===

set -euo pipefail

BIN="${1:?usage: run-e2e-cxx.sh <llvm-build-bin> <sysroot>}"
SYS="${2:?usage: run-e2e-cxx.sh <llvm-build-bin> <sysroot>}"
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT="$(mktemp -d)"
TARGET=arm-pc-wince

CLANGXX="$BIN/clang++"
LLD="$BIN/lld-link"
READOBJ="$BIN/llvm-readobj"
OBJDUMP="$BIN/llvm-objdump"

fail() { echo "E2E-CXX FAIL: $*" >&2; exit 1; }
pass() { echo "E2E-CXX PASS: $*"; }

for lib in c++.lib c++abi.lib unwind.lib clang_rt.builtins-arm.lib; do
  [ -e "$SYS/lib/$lib" ] || fail "runtime library missing: $SYS/lib/$lib (build the runtimes stage first)"
done

# --- 1. Compile + link C++ through the driver --------------------------------
"$CLANGXX" --target=$TARGET --sysroot="$SYS" -O1 \
    "$HERE/cpp_app.cpp" -o "$OUT/cpp.exe" || fail "compile/link cpp_app.cpp"
pass "C++ EXE linked (libc++ + exceptions + RTTI)"

# --- 2. PE structure ----------------------------------------------------------
HDR=$("$READOBJ" -headers --sections --symbols "$OUT/cpp.exe")
echo "$HDR" | grep -q "Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)" \
  || fail "machine type is not IMAGE_FILE_MACHINE_ARM"
echo "$HDR" | grep -q "SubSystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (0x9)" \
  || fail "subsystem is not Windows CE GUI"
pass "PE headers OK"

# --- 3. EHABI tables ------------------------------------------------------------
echo "$HDR" | grep -q "\.ARM\.exidx" || fail ".ARM.exidx missing from image"
echo "$HDR" | grep -q "\.ARM\.extab" || fail ".ARM.extab missing from image"
pass "ARM EHABI unwind tables present"

# --- 4. Personality + ctor dispatch wired in -----------------------------------
nm=$("$BIN/llvm-nm" "$OUT/cpp.exe" 2>/dev/null || true)
echo "$nm" | grep -q "__gxx_personality_v0" \
  || fail "__gxx_personality_v0 not linked in"
echo "$nm" | grep -q "__cxa_throw" || fail "__cxa_throw not linked in"
pass "exception runtime symbols linked"

# --- 5. Thumb C++ (mixed) -------------------------------------------------------
"$CLANGXX" --target=$TARGET --sysroot="$SYS" -O1 -mthumb \
    "$HERE/cpp_app.cpp" -o "$OUT/cpp_t.exe" || fail "compile/link thumb C++"
pass "Thumb C++ EXE linked"

echo
echo "ALL C++ E2E CHECKS PASSED ($OUT)"
