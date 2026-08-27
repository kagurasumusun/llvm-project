#!/bin/bash
#===-- wince-crt/scripts/build-runtimes.sh - WinCE stage-2+3 --------------===//
#
# Stages 2 and 3 of the WinCE toolchain build (see docs/BUILDING.md):
#
#   stage 2 - the WinCE sysroot (import libraries, CRT objects, CRT
#             supplement, headers) from the mingwrt/w32api submodules;
#   stage 3 - cross-build compiler-rt builtins for arm-pc-wince
#             (clang_rt.builtins-arm) and the static C++ runtime stack
#             libunwind + libc++abi + libc++, then stage the results
#             into the sysroot under the names the clang driver's
#             default link line expects (clang_rt.builtins-arm.lib,
#             unwind.lib, c++abi.lib, c++.lib).
#
# The final complete sysroot is published at <prefix>/wince-sysroot,
# where the clang driver finds it by default.
#
# Usage:
#   build-runtimes.sh <stage1-prefix> <source-root> [build-dir]
#
# Environment assumed:
#   <prefix>/bin/clang, clang++, lld-link, llvm-ar, llvm-ranlib, llvm-dlltool
#===------------------------------------------------------------------------===//

set -euo pipefail

PREFIX="${1:?usage: build-runtimes.sh <stage1-prefix> <source-root> [build-dir]}"
SRC="${2:?usage: build-runtimes.sh <stage1-prefix> <source-root> [build-dir]}"
BLD="${3:-$SRC/bld3}"

TARGET=arm-pc-wince
CC="$PREFIX/bin/clang"
CXX="$PREFIX/bin/clang++"
AR="$PREFIX/bin/llvm-ar"
RANLIB="$PREFIX/bin/llvm-ranlib"

[ -x "$CC" ]   || { echo "stage-1 clang missing: $CC" >&2; exit 1; }
[ -x "$CXX" ]  || { echo "stage-1 clang++ missing: $CXX" >&2; exit 1; }

mkdir -p "$BLD"

# --- stage 2 (if not already present): the cross compiler resolves headers
# and CRT from <prefix>/wince-sysroot automatically.
if [ ! -d "$PREFIX/wince-sysroot/lib" ]; then
  cmake -S "$SRC/wince-crt" -B "$BLD/sysroot" -G Ninja \
    -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
    -DLLVM_TOOLS_DIR="$PREFIX/bin" \
    -DCMAKE_C_COMPILER="$CC" \
    -DCMAKE_C_COMPILER_TARGET=$TARGET
  cmake --build "$BLD/sysroot" --target wince-crt
  cp -r "$BLD/sysroot/wince-sysroot" "$PREFIX/wince-sysroot"
fi
[ -d "$PREFIX/wince-sysroot/lib" ] || { echo "stage-2 sysroot build failed" >&2; exit 1; }

# --- builtins ---------------------------------------------------------------
cmake -S "$SRC/compiler-rt/lib/builtins" -B "$BLD/builtins" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_C_COMPILER_TARGET=$TARGET \
  -DCMAKE_ASM_COMPILER_TARGET=$TARGET \
  -DCMAKE_AR="$AR" -DCMAKE_RANLIB="$RANLIB" \
  -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
  -DCOMPILER_RT_BAREMETAL_BUILD=ON \
  -DCMAKE_INSTALL_PREFIX="$PREFIX"
cmake --build "$BLD/builtins"

# --- libunwind + libc++abi + libc++ (static) ---------------------------------
cmake -S "$SRC/runtimes" -B "$BLD/runtimes" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX" \
  -DCMAKE_C_COMPILER_TARGET=$TARGET -DCMAKE_CXX_COMPILER_TARGET=$TARGET \
  -DCMAKE_AR="$AR" -DCMAKE_RANLIB="$RANLIB" \
  -DLLVM_ENABLE_RUNTIMES="libunwind;libcxxabi;libcxx" \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLIBUNWIND_ENABLE_SHARED=OFF -DLIBUNWIND_ENABLE_STATIC=ON \
  -DLIBCXXABI_ENABLE_SHARED=OFF -DLIBCXXABI_ENABLE_STATIC=ON \
  -DLIBCXX_ENABLE_SHARED=OFF -DLIBCXX_ENABLE_STATIC=ON \
  -DLIBCXX_STATICALLY_LINK_ABI_IN_STATIC_LIBRARY=ON \
  -DLIBCXXABI_ENABLE_PIC=OFF -DLIBUNWIND_ENABLE_PIC=OFF -DLIBCXX_ENABLE_PIC=OFF \
  -DLIBCXX_ENABLE_MONOTONIC_CLOCK=ON \
  -DLIBCXX_HAS_PTHREAD_API=ON \
  -DLIBCXX_ENABLE_FILESYSTEM=OFF \
  -DLIBCXX_ENABLE_WIDE_CHARACTERS=ON \
  -DLIBCXXABI_USE_COMPILER_RT=ON -DLIBUNWIND_USE_COMPILER_RT=ON \
  -DLIBCXXABI_ENABLE_EXCEPTIONS=ON \
  -DCMAKE_INSTALL_PREFIX="$PREFIX"
cmake --build "$BLD/runtimes"

# --- stage everything into the sysroot ---------------------------------------
# wince-crt's staging step copies each library under the driver-expected
# name; WINCE_RUNTIME_LIBS is a single directory, so gather the products
# of both builds there first.
BUILTIN_LIB=$(find "$BLD/builtins" -name 'clang_rt.builtins-arm*' | head -1)
STAGE_DIR="$BLD/stage"
mkdir -p "$STAGE_DIR"
cp -f "$BUILTIN_LIB" "$STAGE_DIR/"
for l in libunwind libc++abi libc++; do
  cp -f "$(find "$BLD/runtimes" -name "$l.a" | head -1)" "$STAGE_DIR/"
done

cmake -S "$SRC/wince-crt" -B "$BLD/stage-crt" \
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY \
  -DLLVM_TOOLS_DIR="$PREFIX/bin" \
  -DCMAKE_C_COMPILER="$CC" -DCMAKE_C_COMPILER_TARGET=$TARGET \
  -DWINCE_RUNTIME_LIBS="$STAGE_DIR"
cmake --build "$BLD/stage-crt" --target wince-crt

# Publish the complete sysroot (CRT + runtimes) under the prefix, where
# the clang driver finds it by default.
rm -rf "$PREFIX/wince-sysroot"
cp -r "$BLD/stage-crt/wince-sysroot" "$PREFIX/wince-sysroot"

echo "WinCE sysroot + runtimes built and staged:"
ls -l "$PREFIX/wince-sysroot/lib" | grep -E 'clang_rt|unwind|c\+\+'
