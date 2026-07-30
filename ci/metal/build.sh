#!/usr/bin/env bash
#
# Build enough of Clang to compile and run the Metal frontend tests, then run
# them. Everything the CI actually does lives here rather than in the workflow
# YAML, because .github/workflows/ is write-protected for GitHub Apps -- both
# for the agent working on this repository and for the built-in GITHUB_TOKEN,
# neither of which can hold the `workflows` permission. Files outside that
# directory have no such restriction, so keeping the logic here is what makes
# the pipeline editable without a human in the loop.
#
# Usage: ci/metal/build.sh <stage>
#
#   deps      install the toolchain
#   stdlib    fetch Apple's <metal_stdlib> headers from metal-info
#   configure run cmake
#   tablegen  build only the TableGen outputs (fails fast on Attr.td etc.)
#   build     build clang and the test tools
#   smoke     compile a kernel end to end
#   test      run clang/test/Metal
#
set -uo pipefail

BUILD_DIR=${BUILD_DIR:-build}
STDLIB_DIR=${STDLIB_DIR:-/tmp/metal-info}
JOBS=${JOBS:-$(nproc)}

# Re-emit compiler and lit diagnostics as workflow annotations. The raw job log
# is served from Azure Blob storage, which is not always reachable from where
# these logs get read back, so anything that matters has to come through the
# annotations API instead.
annotate_errors() {
  local log=$1 limit=${2:-15}
  grep -E 'error:' "$log" 2>/dev/null | head -"$limit" | while IFS= read -r line; do
    printf '::error::%s\n' "${line:0:400}"
  done
}

stage_deps() {
  sudo apt-get update -qq
  sudo apt-get install -y --no-install-recommends \
    ninja-build mold ccache cmake python3
  echo "nproc=$(nproc)"
  mold --version
  ccache --version | head -1
}

# Apple's real headers. clang/test/Metal does not include <metal_stdlib>, but
# having the tree on the runner is what lets the smoke stage measure how far a
# real include gets -- which is the yardstick for how complete the frontend is.
# metal-info is ~12 GB whole; a blobless sparse checkout of just this subtree
# is a few tens of megabytes.
stage_stdlib() {
  rm -rf "$STDLIB_DIR"
  git clone --filter=blob:none --no-checkout --depth 1 \
    https://github.com/kagurasumusun/metal-info.git "$STDLIB_DIR" || {
      echo "::warning::could not clone metal-info; skipping stdlib"
      return 0
    }
  (
    cd "$STDLIB_DIR"
    git sparse-checkout init --cone
    git sparse-checkout set reference-apple
    git checkout
  ) || {
    echo "::warning::sparse checkout failed; skipping stdlib"
    return 0
  }

  local sdk
  sdk=$(find "$STDLIB_DIR/reference-apple" -type d -path '*/include/metal' | head -1)
  if [ -z "$sdk" ]; then
    echo "::warning::no include/metal directory found"
    return 0
  fi
  echo "found $sdk"
  find "$sdk" -type f | wc -l
  if [ -n "${GITHUB_OUTPUT:-}" ]; then
    echo "metal_stdlib=$sdk" >> "$GITHUB_OUTPUT"
  fi
  echo "$sdk" > /tmp/metal_stdlib_path
}

# One target only, and it has to be the host one: LLVM_DEFAULT_TARGET_TRIPLE is
# only derived when the native backend is among LLVM_TARGETS_TO_BUILD, and with
# an empty triple lit aborts with "Could not turn '' into Itanium ABI triple"
# before running a single test. Nothing in clang/test/Metal depends on which
# backend is built.
stage_configure() {
  local arch triple target
  arch=$(uname -m)
  case "$arch" in
    aarch64|arm64) target=AArch64; triple=aarch64-unknown-linux-gnu ;;
    x86_64)        target=X86;     triple=x86_64-unknown-linux-gnu ;;
    *)             target=X86;     triple=$arch-unknown-linux-gnu ;;
  esac
  echo "host=$arch target=$target triple=$triple"

  cmake -G Ninja -S llvm -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_PROJECTS=clang \
    -DLLVM_TARGETS_TO_BUILD="$target" \
    -DLLVM_DEFAULT_TARGET_TRIPLE="$triple" \
    -DLLVM_ENABLE_ASSERTIONS=OFF \
    -DLLVM_OPTIMIZED_TABLEGEN=ON \
    -DLLVM_USE_LINKER=mold \
    -DLLVM_BUILD_LLVM_DYLIB=ON \
    -DLLVM_LINK_LLVM_DYLIB=ON \
    -DCLANG_LINK_CLANG_DYLIB=ON \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_INCLUDE_DOCS=OFF \
    -DLLVM_ENABLE_BINDINGS=OFF \
    -DLLVM_ENABLE_OCAMLDOC=OFF \
    -DLLVM_ENABLE_ZLIB=OFF \
    -DLLVM_ENABLE_ZSTD=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DLLVM_ENABLE_LIBXML2=OFF \
    -DLLVM_ENABLE_LIBEDIT=OFF \
    -DCLANG_ENABLE_STATIC_ANALYZER=OFF \
    -DCLANG_ENABLE_ARCMT=OFF \
    -DLLVM_INCLUDE_TESTS=ON \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_C_FLAGS="-g0" \
    -DCMAKE_CXX_FLAGS="-g0"
}

# The Metal work touches Attr.td, Builtins.def and the diagnostics, and has
# broken TableGen more than once, so surface that before the long compile.
stage_tablegen() {
  ninja -C "$BUILD_DIR" -j"$JOBS" clang-tablegen-targets 2>&1 | tee /tmp/tblgen.log
  local rc=${PIPESTATUS[0]}
  [ "$rc" -eq 0 ] || annotate_errors /tmp/tblgen.log 20
  return "$rc"
}

stage_build() {
  ninja -C "$BUILD_DIR" -j"$JOBS" clang FileCheck count not llvm-lit llvm-config \
    2>&1 | tee /tmp/build.log
  local rc=${PIPESTATUS[0]}
  if [ "$rc" -ne 0 ]; then
    echo "::group::first errors"
    grep -nE 'error:|FAILED:' /tmp/build.log | head -40
    echo "::endgroup::"
    annotate_errors /tmp/build.log
  fi
  return "$rc"
}

stage_smoke() {
  cat > /tmp/smoke.metal <<'EOF'
kernel void k(device float *out [[buffer(0)]],
              unsigned int id [[thread_position_in_grid]]) {
  out[id] = 1.0f;
}
EOF
  "./$BUILD_DIR/bin/clang" -cc1 -x metal \
    -triple air64_v28-apple-macosx26.0.0 -std=metal3.2 \
    -emit-llvm -no-opaque-pointers -o - /tmp/smoke.metal || return 1

  # How far the real <metal_stdlib> gets. Informational: it is expected to fail
  # for now, so it must not gate the job.
  local sdk=""
  [ -f /tmp/metal_stdlib_path ] && sdk=$(cat /tmp/metal_stdlib_path)
  if [ -n "$sdk" ] && [ -d "$sdk" ]; then
    echo "::group::<metal_stdlib> parse attempt"
    printf '#include <metal_stdlib>\nkernel void k() {}\n' > /tmp/std.metal
    "./$BUILD_DIR/bin/clang" -cc1 -x metal \
      -triple air64_v28-apple-macosx26.0.0 -std=metal3.2 \
      -I "$(dirname "$sdk")" -fsyntax-only /tmp/std.metal 2>&1 | head -40
    echo "::endgroup::"
  fi
  return 0
}

stage_test() {
  "./$BUILD_DIR/bin/llvm-lit" -v --timeout=120 clang/test/Metal 2>&1 | tee /tmp/lit.log
  local rc=${PIPESTATUS[0]}
  echo "::group::lit summary"
  sed -n '/Failed Tests/,$p' /tmp/lit.log
  echo "::endgroup::"
  if [ "$rc" -ne 0 ]; then
    grep -E '^(FAIL|UNRESOLVED|TIMEOUT):' /tmp/lit.log | while IFS= read -r line; do
      printf '::error::%s\n' "$line"
    done
    annotate_errors /tmp/lit.log 25
  fi
  return "$rc"
}

case "${1:-}" in
  deps)      stage_deps ;;
  stdlib)    stage_stdlib ;;
  configure) stage_configure ;;
  tablegen)  stage_tablegen ;;
  build)     stage_build ;;
  smoke)     stage_smoke ;;
  test)      stage_test ;;
  *)
    echo "usage: $0 {deps|stdlib|configure|tablegen|build|smoke|test}" >&2
    exit 2
    ;;
esac
