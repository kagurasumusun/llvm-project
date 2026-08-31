#!/bin/bash
#===- utils/wince/build-wince-runtimes.sh - WinCE stage-3 ----------------===//
#
# Stage 3 of the WinCE toolchain build (after
# utils/wince/build-wince-sysroot.sh assembled the mingwrt/w32api/pthreads
# sysroot): cross-build compiler-rt builtins for arm-pc-wince and the
# static C++ runtime stack libunwind + libc++abi + libc++, and stage the
# results into the sysroot under the GNU names the clang driver's default
# link line probes:
#
#   libclang_rt.builtins-arm.a   (the -lgcc replacement)
#   libunwind.a  libc++abi.a  libc++
#
# libc++ uses the pthread threading API on this target, served by the
# pthreads4w static library (libpthread.a) built by the sysroot stage.
#
# Usage:
#   build-wince-runtimes.sh --toolchain <dir> [--sysroot <dir>] \
#       [--target arm-pc-wince] [--build-dir <dir>]
#
#===------------------------------------------------------------------------===//

set -euo pipefail

PROGRAM="$(basename "$0")"
REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"

TARGET="arm-pc-wince"
TOOLCHAIN=""
SYSROOT=""
BLD=""

while [ $# -gt 0 ]; do
  case "$1" in
    --toolchain) TOOLCHAIN="$2"; shift 2 ;;
    --sysroot)   SYSROOT="$2"; shift 2 ;;
    --target)    TARGET="$2"; shift 2 ;;
    --build-dir) BLD="$2"; shift 2 ;;
    -h|--help)   sed -n '2,32p' "$0" | sed 's/^# \{0,1\}//'; exit 1 ;;
    *) echo "$PROGRAM: unknown option: $1" >&2; exit 1 ;;
  esac
done

[ -n "$TOOLCHAIN" ] || { echo "$PROGRAM: --toolchain is required" >&2; exit 1; }
[ -x "$TOOLCHAIN/clang" ] || { echo "$PROGRAM: $TOOLCHAIN/clang missing" >&2; exit 1; }
if [ -z "$SYSROOT" ]; then
  SYSROOT="$TOOLCHAIN/../wince-sysroot"
fi
[ -d "$SYSROOT/lib" ] || {
  echo "$PROGRAM: no sysroot at $SYSROOT; run build-wince-sysroot.sh first" >&2
  exit 1
}
[ -n "$BLD" ] || BLD="$REPO_ROOT/build-wince-runtimes"
mkdir -p "$BLD"

CC="$TOOLCHAIN/clang"
CXX="$TOOLCHAIN/clang++"
AR="$TOOLCHAIN/llvm-ar"
RANLIB="$TOOLCHAIN/llvm-ranlib"

case "$TARGET" in
  arm*) RT_ARCH="arm" ;;
  i386*) RT_ARCH="i386" ;;
  *) echo "$PROGRAM: unsupported target $TARGET" >&2; exit 1 ;;
esac

# CMAKE_<LANG>_COMPILER_TARGET is ignored until CMake has identified the
# compiler as Clang.  Identification compiles do not pass it, so put
# --target in the language flags (CI 33352687660).
COMMON_CMAKE=(
  -G Ninja
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
  -DCMAKE_AR="$AR" -DCMAKE_RANLIB="$RANLIB"
  -DCMAKE_C_COMPILER="$CC"
  -DCMAKE_CXX_COMPILER="$CXX"
  -DCMAKE_ASM_COMPILER="$CC"
  -DCMAKE_C_COMPILER_TARGET="$TARGET"
  -DCMAKE_CXX_COMPILER_TARGET="$TARGET"
  -DCMAKE_ASM_COMPILER_TARGET="$TARGET"
)

# --- compiler-rt builtins (the -lgcc replacement) ----------------------------
echo "== [1/2] compiler-rt builtins ($RT_ARCH)"
# Builtins compile against the compiler's own headers only.  The WinCE
# driver still injects <sysroot>/include (default next to the prefix),
# so mingwrt's float.h is include_next'd from clang's and then has
# nothing left (CI 33353690290).  -nostdlibinc drops the CRT headers.
cmake -S "$REPO_ROOT/compiler-rt/lib/builtins" -B "$BLD/builtins" \
  "${COMMON_CMAKE[@]}" \
  -DCMAKE_C_FLAGS="--target=$TARGET -nostdlibinc" \
  -DCMAKE_CXX_FLAGS="--target=$TARGET -nostdlibinc" \
  -DCMAKE_ASM_FLAGS="--target=$TARGET -nostdlibinc" \
  -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
  -DCOMPILER_RT_BAREMETAL_BUILD=ON
cmake --build "$BLD/builtins" -j "$(nproc 2>/dev/null || echo 2)"

# --- libunwind + libc++abi + libc++ (static) ---------------------------------
echo "== [2/2] libunwind + libc++abi + libc++"
cmake -S "$REPO_ROOT/runtimes" -B "$BLD/runtimes" \
  "${COMMON_CMAKE[@]}" \
  -DCMAKE_SYSROOT="$SYSROOT" \
  -DCMAKE_C_FLAGS="--target=$TARGET --sysroot=$SYSROOT" \
  -DCMAKE_CXX_FLAGS="--target=$TARGET --sysroot=$SYSROOT" \
  -DCMAKE_ASM_FLAGS="--target=$TARGET --sysroot=$SYSROOT" \
  -DLLVM_ENABLE_RUNTIMES="libunwind;libcxxabi;libcxx" \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLIBUNWIND_ENABLE_SHARED=OFF -DLIBUNWIND_ENABLE_STATIC=ON \
  -DLIBUNWIND_ENABLE_PIC=OFF \
  -DLIBCXXABI_ENABLE_SHARED=OFF -DLIBCXXABI_ENABLE_STATIC=ON \
  -DLIBCXXABI_ENABLE_PIC=OFF \
  -DLIBCXXABI_USE_COMPILER_RT=ON \
  -DLIBUNWIND_USE_COMPILER_RT=ON \
  -DLIBCXXABI_ENABLE_EXCEPTIONS=ON \
  -DLIBCXX_ENABLE_SHARED=OFF -DLIBCXX_ENABLE_STATIC=ON \
  -DLIBCXX_ENABLE_PIC=OFF \
  -DLIBCXX_STATICALLY_LINK_ABI_IN_STATIC_LIBRARY=ON \
  -DLIBCXX_ENABLE_MONOTONIC_CLOCK=ON \
  -DLIBCXX_HAS_PTHREAD_API=ON \
  -DLIBCXX_ENABLE_FILESYSTEM=OFF \
  -DLIBCXX_ENABLE_WIDE_CHARACTERS=ON

# Configure first, then build (the runtimes umbrella configures sub-builds).
cmake --build "$BLD/runtimes" -j "$(nproc 2>/dev/null || echo 2)" || true
for sub in libunwind libcxxabi libcxx; do
  if [ -d "$BLD/runtimes/$sub" ]; then
    cmake --build "$BLD/runtimes/$sub" -j "$(nproc 2>/dev/null || echo 2)"
  fi
done

# --- stage into the sysroot (GNU names; the driver probes lib<name>.a) -------
install -m 644 \
  "$(find "$BLD/builtins" -name "libclang_rt.builtins-$RT_ARCH.a" -o -name "clang_rt.builtins-$RT_ARCH.lib" | head -1)" \
  "$SYSROOT/lib/libclang_rt.builtins-$RT_ARCH.a"
for lib in libunwind libc++abi libc++; do
  install -m 644 "$(find "$BLD/runtimes" -name "$lib.a" | head -1)" \
    "$SYSROOT/lib/$lib.a"
done
# C++ headers for AddClangCXXStdlibIncludeArgs (<sysroot>/include/c++/v1).
if [ -d "$BLD/runtimes/include/c++/v1" ]; then
  mkdir -p "$SYSROOT/include/c++"
  cp -r "$BLD/runtimes/include/c++/v1" "$SYSROOT/include/c++/"
fi

echo "== done:"
ls -l "$SYSROOT/lib" | grep -E 'clang_rt|unwind|c\+\+'
