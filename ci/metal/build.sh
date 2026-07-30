#!/usr/bin/env bash
#
# Build the minimum needed to run clang/test/Metal, and run it.
#
# Everything the CI does lives here rather than in the workflow YAML, because
# .github/workflows/ is write-protected for GitHub Apps. Keeping the logic in a
# normal file means the pipeline can be changed without editing a protected
# one; it also means every stage can be run locally, unchanged.
#
# Usage: ci/metal/build.sh <stage>
#
#   deps        install the toolchain
#   prune-cache evict old Actions cache entries to stay under the 10 GB cap
#   stdlib      fetch Apple's <metal_stdlib> headers from metal-info
#   configure   run cmake
#   build       build clang and the two tools lit insists on
#   smoke       compile a kernel end to end
#   test        run clang/test/Metal
#
# What is deliberately *not* built:
#
#   * Any target backend. LLVM_TARGETS_TO_BUILD is empty. The Metal frontend
#     emits AIR as LLVM IR and stops there -- no backend is ever invoked, and
#     none of clang/test/Metal depends on one. This is the single biggest win:
#     a backend is hundreds of files of TableGen output and object code.
#     LLVM_DEFAULT_TARGET_TRIPLE has to be set explicitly, because it is
#     otherwise derived only when the native backend is present, and an empty
#     triple makes lit abort before it runs anything.
#   * A separate TableGen stage. `ninja clang` already orders the .inc files
#     ahead of everything that includes them; asking for them first only added
#     a second ninja invocation.
#   * llvm-lit. cmake writes build/bin/llvm-lit at configure time; it is not a
#     ninja target and asking for it fails the build.
#   * llvm-config, opt, llc, and the rest of the tool suite. Nothing in
#     clang/test/Metal shells out to them.
#
# `not` and `count` *are* built even though no Metal test uses them: lit
# registers them with unresolved='fatal', so it refuses to start without them.
#
set -uo pipefail

BUILD_DIR=${BUILD_DIR:-build}
STDLIB_DIR=${STDLIB_DIR:-/tmp/metal-info}
JOBS=${JOBS:-$(nproc)}
# Links are memory hungry. The hosted ARM runner has 4 vCPUs but 16 GB, and
# linking several LLVM shared libraries at once exhausted it -- the build died
# with exit 139 (SIGSEGV) partway through. Compiles stay at full width.
LINK_JOBS=${LINK_JOBS:-1}

# Re-emit diagnostics as workflow annotations. The raw job log is served from
# Azure Blob storage, which is not always reachable from where these logs get
# read back, so anything that matters has to come through the annotations API.
annotate_errors() {
  local log=$1 limit=${2:-15}
  [ -f "$log" ] || return 0
  grep -E 'error:' "$log" 2>/dev/null | head -"$limit" | while IFS= read -r line; do
    printf '::error::%s\n' "${line:0:400}"
  done
}

stage_deps() {
  sudo apt-get update -qq
  # mold is the linker; lld would mean building it first, and mold is faster
  # on this workload anyway. ccache and ninja are the other two that matter.
  # No cmake here: the runner image already ships a current one.
  sudo apt-get install -y --no-install-recommends ninja-build mold ccache
  echo "cpus=$(nproc) mem=$(free -g | awk '/^Mem:/{print $2}')GB"
  mold --version
  ccache --version | head -1
}

# The repository-wide Actions cache is capped at 10 GB. It was sitting at
# 9.8 GB of entries left by other branches, which is why every save from this
# workflow failed with "Cache save failed" and the build never got a warm
# cache. Evict least-recently-used entries until there is headroom.
#
# Needs actions:write, which GITHUB_TOKEN has and the agent's App installation
# does not, so this can only run from inside a job.
stage_prune_cache() {
  local repo=${GITHUB_REPOSITORY:-kagurasumusun/llvm-project}
  local budget=$(( ${CACHE_BUDGET_GB:-6} * 1024 * 1024 * 1024 ))

  command -v gh >/dev/null 2>&1 || { echo "gh unavailable; skipping"; return 0; }

  local used
  used=$(gh api "repos/$repo/actions/cache/usage" \
           --jq '.active_caches_size_in_bytes' 2>/dev/null) || return 0
  echo "cache in use: $((used / 1024 / 1024)) MB, budget $((budget / 1024 / 1024)) MB"
  [ "$used" -lt "$budget" ] && { echo "under budget"; return 0; }

  gh api "repos/$repo/actions/caches?per_page=100" --paginate \
    --jq '.actions_caches[] | [.id, .size_in_bytes, .last_accessed_at, .ref] | @tsv' \
    2>/dev/null | sort -k3 > /tmp/caches.tsv

  while IFS=$'\t' read -r id size accessed ref; do
    [ "$used" -lt "$budget" ] && break
    # Never evict an entry belonging to the branch being built.
    case "$ref" in *"${GITHUB_REF_NAME:-__none__}"*) continue ;; esac
    if gh api -X DELETE "repos/$repo/actions/caches/$id" >/dev/null 2>&1; then
      used=$((used - size))
      echo "evicted $((size / 1024 / 1024)) MB from ${ref##*/}"
    fi
  done < /tmp/caches.tsv
  echo "cache now: $((used / 1024 / 1024)) MB"
  return 0
}

# Apple's real headers. clang/test/Metal does not include <metal_stdlib>, but
# having the tree on the runner is what lets the smoke stage measure how far a
# real include gets, which is the yardstick for how complete the frontend is.
# metal-info is ~12 GB whole; a blobless sparse checkout of this subtree is a
# few tens of megabytes.
stage_stdlib() {
  rm -rf "$STDLIB_DIR"
  git clone --filter=blob:none --no-checkout --depth 1 --single-branch \
    https://github.com/kagurasumusun/metal-info.git "$STDLIB_DIR" 2>/dev/null || {
      echo "::warning::could not clone metal-info; skipping stdlib"
      return 0
    }
  (
    cd "$STDLIB_DIR"
    git sparse-checkout init --cone
    git sparse-checkout set reference-apple
    git checkout
  ) >/dev/null 2>&1 || {
    echo "::warning::sparse checkout failed; skipping stdlib"
    return 0
  }

  local sdk
  sdk=$(find "$STDLIB_DIR/reference-apple" -type d -path '*/include/metal' | head -1)
  [ -n "$sdk" ] || { echo "::warning::no include/metal found"; return 0; }
  echo "$sdk ($(find "$sdk" -type f | wc -l) files)"
  echo "$sdk" > /tmp/metal_stdlib_path
}

stage_configure() {
  # The triple only has to be well-formed; no backend stands behind it.
  local triple="${LLVM_TRIPLE:-$(uname -m)-unknown-linux-gnu}"
  echo "triple=$triple targets=<none>"

  cmake -G Ninja -S llvm -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_PROJECTS=clang \
    -DLLVM_TARGETS_TO_BUILD="" \
    -DLLVM_DEFAULT_TARGET_TRIPLE="$triple" \
    -DLLVM_ENABLE_ASSERTIONS=OFF \
    -DLLVM_OPTIMIZED_TABLEGEN=ON \
    -DLLVM_USE_LINKER=mold \
    -DLLVM_PARALLEL_LINK_JOBS="$LINK_JOBS" \
    -DLLVM_BUILD_LLVM_DYLIB=ON \
    -DLLVM_LINK_LLVM_DYLIB=ON \
    -DCLANG_LINK_CLANG_DYLIB=ON \
    -DLLVM_INCLUDE_TESTS=ON \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_INCLUDE_BENCHMARKS=OFF \
    -DLLVM_INCLUDE_DOCS=OFF \
    -DLLVM_INCLUDE_UTILS=ON \
    -DLLVM_BUILD_TOOLS=OFF \
    -DLLVM_BUILD_UTILS=OFF \
    -DLLVM_BUILD_RUNTIME=OFF \
    -DLLVM_ENABLE_BINDINGS=OFF \
    -DLLVM_ENABLE_OCAMLDOC=OFF \
    -DLLVM_ENABLE_PLUGINS=OFF \
    -DLLVM_ENABLE_LIBPFM=OFF \
    -DLLVM_ENABLE_Z3_SOLVER=OFF \
    -DLLVM_ENABLE_HTTPLIB=OFF \
    -DLLVM_ENABLE_CURL=OFF \
    -DLLVM_ENABLE_ZLIB=OFF \
    -DLLVM_ENABLE_ZSTD=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DLLVM_ENABLE_LIBXML2=OFF \
    -DLLVM_ENABLE_LIBEDIT=OFF \
    -DCLANG_BUILD_TOOLS=ON \
    -DCLANG_ENABLE_STATIC_ANALYZER=OFF \
    -DCLANG_ENABLE_ARCMT=OFF \
    -DCLANG_ENABLE_OBJC_REWRITER=OFF \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_C_FLAGS="-g0" \
    -DCMAKE_CXX_FLAGS="-g0"
}

# One ninja invocation for everything. `clang` pulls in the TableGen outputs
# it needs, in the right order, on its own.
stage_build() {
  ninja -C "$BUILD_DIR" -j"$JOBS" clang FileCheck not count 2>&1 | tee /tmp/build.log
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

  # How far the real <metal_stdlib> gets. Informational: it is expected to
  # fail for now, so it must not gate the job.
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
  local lit="./$BUILD_DIR/bin/llvm-lit"
  if [ ! -x "$lit" ]; then
    echo "::warning::${lit} missing; using llvm/utils/lit/lit.py"
    lit="python3 llvm/utils/lit/lit.py"
  fi
  $lit -v --timeout=120 clang/test/Metal 2>&1 | tee /tmp/lit.log
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
  deps)         stage_deps ;;
  prune-cache)  stage_prune_cache ;;
  stdlib)       stage_stdlib ;;
  configure)    stage_configure ;;
  build)        stage_build ;;
  smoke)        stage_smoke ;;
  test)         stage_test ;;
  *)
    echo "usage: $0 {deps|prune-cache|stdlib|configure|build|smoke|test}" >&2
    exit 2
    ;;
esac
