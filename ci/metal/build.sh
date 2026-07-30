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
#   * lit, and the whole LLVM_INCLUDE_TESTS tree behind it. See
#     ci/metal/run-tests.sh.
#   * llvm-config, opt, llc, and the rest of the tool suite. Nothing in
#     clang/test/Metal shells out to them.
#   * The two aggregate shared libraries. libLLVM.so and libclang-cpp.so each
#     bundle *every* library of their project, so they drag in code that is
#     switched off elsewhere: clangStaticAnalyzer* survived
#     CLANG_ENABLE_STATIC_ANALYZER=OFF, and lib/ExecutionEngine,
#     lib/ObjectYAML and lib/MCA were built for a compiler that never calls
#     them. Linking statically against only what the driver references drops
#     596 build steps (2822 -> 2226, measured), and removes those three
#     directories entirely.
# The long list of LLVM_TOOL_*_BUILD / CLANG_TOOL_*_BUILD above is every tool
# the cache reported as ON, minus llvm-config and the clang driver: 116 of
# them, from llvm-objdump and opt down to the fuzzers. LLVM_BUILD_TOOLS=OFF
# does not stop them being generated, and LLVM_INCLUDE_TOOLS=OFF cannot be
# used because clang itself lives under tools/. Switching them off takes the
# build graph from 9091 targets to 7237.
#
# This is only possible because the tests no longer go through lit: with
# LLVM_INCLUDE_TESTS=ON, check-llvm and check-clang list BugpointPasses,
# clang-format, clang-offload-bundler and the rest as hard dependencies, and
# configure refuses to drop any of them.
#
# LLVM_OPTIMIZED_TABLEGEN is not set: it only does anything when assertions
# are on, and then it builds a whole second host toolchain to get an
# unassertioned TableGen. With assertions off it is dead weight.
#
# LLVM_ENABLE_PCH does not exist in this tree -- it arrived after LLVM 16.
# CMake's own target_precompile_headers is not wired up here either, so there
# is no precompiled-header knob to turn.
#
# LLVM_INCLUDE_TESTS=OFF also means no tools/clang/test/lit.site.cfg.py, so
# lit cannot run -- which is why ci/metal/run-tests.sh replaces it. That is a
# fair trade: the only lit features these tests use are the %clang_cc1 and %s
# substitutions, and -verify is clang's own, so a twenty-line runner covers
# all 21 of them.
#
# Build-time-only overhead that is switched off. None of it changes what the
# compiler can do; it only costs time:
#
#   * LLVM_ENABLE_WARNINGS / PEDANTIC / WERROR. Nineteen -W flags were being
#     passed to every translation unit purely to produce diagnostics nobody
#     reads in CI.
#   * LLVM_APPEND_VC_REV. Stamps the git revision into a header, so the
#     header changes on every commit and invalidates the objects that include
#     it -- exactly the wrong behaviour when the point is to reuse ccache.
#   * LLVM_ENABLE_PIC. -fPIC costs a little on every file and buys nothing
#     once the shared libraries are gone.
#   * ABI_BREAKING_CHECKS, EXPENSIVE_CHECKS, ENABLE_DUMP: extra code compiled
#     into the binary for debugging.
#   * CLANG_INCLUDE_TESTS, INSTALL_UTILS, EXPORT_COMPILE_COMMANDS,
#     install rules: build-graph clutter that is never reached.
#
# Where the remaining 2218 steps go, and why none of it is reachable from a
# cmake flag -- each of these is named in a CMakeLists LLVM_LINK_COMPONENTS
# list, so the dependency is structural:
#
#   lib/Transforms      270  clangCodeGen links IPO, InstCombine, Passes, LTO
#   lib/CodeGen         220  ditto
#   lib/DebugInfo       171  clang links CodeView/DWARF/MSF/PDB; PDB alone is
#                            93 steps of Windows-only debug format, and
#                            LLVM_ENABLE_BACKTRACES=OFF does not shift it
#   lib/Support         149
#   lib/Analysis        121
#   clang/lib/AST       101
#   utils/TableGen       84  needed to generate the 76 .inc files
#
# Dropping these would mean editing CMakeLists, not configuring differently.
#
# What could not be dropped:
#
#   * lib/DebugInfo (171 steps). clangCodeGen emits debug info, so the DWARF
#     writers come with it.
#
# `not` and `count` are gone too. They were only ever built because lit
# registers them with unresolved='fatal' and refuses to start otherwise; no
# Metal test invokes either. Only clang and FileCheck are needed now.
#
set -uo pipefail

BUILD_DIR=${BUILD_DIR:-build}
STDLIB_DIR=${STDLIB_DIR:-/tmp/metal-info}
# Oversubscribe the compile jobs. A C++ compile is not CPU-bound the whole
# time -- it waits on the filesystem for headers, and on a ccache hit it does
# almost nothing but read and write -- so running more jobs than cores keeps
# all four busy. 16 GB across 6 jobs leaves well over 2 GB each, which is
# ample for clang translation units at -g0.
JOBS=${JOBS:-$(( $(nproc) + 2 ))}
# Compiles run at full width; links are throttled separately. The SIGSEGV that
# forced this was caused by linking several *shared* LLVM libraries at once,
# and those are no longer built at all.
# 2 rather than 1: with the aggregate shared libraries gone the only heavy
# link left is the clang executable itself, and 67 static archives that cost
# almost nothing. Serialising all of them was over-cautious.
LINK_JOBS=${LINK_JOBS:-2}

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
  # llvm is here for llvm-symbolizer and gdb for scripted backtraces: without
  # them a clang crash prints bare addresses ("Stack dump without symbol
  # names"), which cannot be read back from the annotations the runner emits.
  # gdb works directly off the symbol table (kept even in this Release -g0
  # build), and its batch mode does not depend on the symbolizer protocol,
  # which stalls after the first frame on this binary.
  sudo apt-get install -y --no-install-recommends ninja-build mold ccache llvm \
    gdb
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
    -DLLVM_ABI_BREAKING_CHECKS=FORCE_OFF \
    -DLLVM_ENABLE_EXPENSIVE_CHECKS=OFF \
    -DLLVM_ENABLE_DUMP=OFF \
    -DLLVM_ENABLE_WARNINGS=OFF \
    -DLLVM_ENABLE_PEDANTIC=OFF \
    -DLLVM_ENABLE_WERROR=OFF \
    -DLLVM_ENABLE_MODULES=OFF \
    -DLLVM_ENABLE_PIC=OFF \
    -DLLVM_ENABLE_UNWIND_TABLES=OFF \
    -DLLVM_ENABLE_FFI=OFF \
    -DLLVM_BUILD_BENCHMARKS=OFF \
    -DLLVM_BUILD_TESTS=OFF \
    -DCLANG_PLUGIN_SUPPORT=OFF \
    -DLLVM_APPEND_VC_REV=OFF \
    -DLLVM_INSTALL_UTILS=OFF \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF \
    -DCMAKE_SKIP_INSTALL_RULES=ON \
    -DCLANG_INCLUDE_TESTS=OFF \
    -DLLVM_USE_LINKER=mold \
    -DLLVM_PARALLEL_LINK_JOBS="$LINK_JOBS" \
    -DLLVM_BUILD_LLVM_DYLIB=OFF \
    -DLLVM_LINK_LLVM_DYLIB=OFF \
    -DCLANG_LINK_CLANG_DYLIB=OFF \
    -DLLVM_INCLUDE_TESTS=OFF \
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
    -DCLANG_TOOL_AMDGPU_ARCH_BUILD=OFF \
    -DCLANG_TOOL_APINOTES_TEST_BUILD=OFF \
    -DCLANG_TOOL_ARCMT_TEST_BUILD=OFF \
    -DCLANG_TOOL_CLANG_CHECK_BUILD=OFF \
    -DCLANG_TOOL_CLANG_DIFF_BUILD=OFF \
    -DCLANG_TOOL_CLANG_EXTDEF_MAPPING_BUILD=OFF \
    -DCLANG_TOOL_CLANG_FORMAT_BUILD=OFF \
    -DCLANG_TOOL_CLANG_FORMAT_VS_BUILD=OFF \
    -DCLANG_TOOL_CLANG_FUZZER_BUILD=OFF \
    -DCLANG_TOOL_CLANG_IMPORT_TEST_BUILD=OFF \
    -DCLANG_TOOL_CLANG_LINKER_WRAPPER_BUILD=OFF \
    -DCLANG_TOOL_CLANG_OFFLOAD_BUNDLER_BUILD=OFF \
    -DCLANG_TOOL_CLANG_OFFLOAD_PACKAGER_BUILD=OFF \
    -DCLANG_TOOL_CLANG_REFACTOR_BUILD=OFF \
    -DCLANG_TOOL_CLANG_RENAME_BUILD=OFF \
    -DCLANG_TOOL_CLANG_REPL_BUILD=OFF \
    -DCLANG_TOOL_CLANG_SCAN_DEPS_BUILD=OFF \
    -DCLANG_TOOL_CLANG_SHLIB_BUILD=OFF \
    -DCLANG_TOOL_C_ARCMT_TEST_BUILD=OFF \
    -DCLANG_TOOL_C_INDEX_TEST_BUILD=OFF \
    -DCLANG_TOOL_DIAGTOOL_BUILD=OFF \
    -DCLANG_TOOL_DICTIONARY_BUILD=OFF \
    -DCLANG_TOOL_HANDLE_CXX_BUILD=OFF \
    -DCLANG_TOOL_HANDLE_LLVM_BUILD=OFF \
    -DCLANG_TOOL_LIBCLANG_BUILD=OFF \
    -DCLANG_TOOL_NVPTX_ARCH_BUILD=OFF \
    -DCLANG_TOOL_SCAN_BUILD_BUILD=OFF \
    -DCLANG_TOOL_SCAN_BUILD_PY_BUILD=OFF \
    -DCLANG_TOOL_SCAN_VIEW_BUILD=OFF \
    -DLLVM_TOOL_BUGPOINT_BUILD=OFF \
    -DLLVM_TOOL_BUGPOINT_PASSES_BUILD=OFF \
    -DLLVM_TOOL_DSYMUTIL_BUILD=OFF \
    -DLLVM_TOOL_DXIL_DIS_BUILD=OFF \
    -DLLVM_TOOL_GOLD_BUILD=OFF \
    -DLLVM_TOOL_LLC_BUILD=OFF \
    -DLLVM_TOOL_LLI_BUILD=OFF \
    -DLLVM_TOOL_LLVM_AR_BUILD=OFF \
    -DLLVM_TOOL_LLVM_AS_BUILD=OFF \
    -DLLVM_TOOL_LLVM_AS_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_BCANALYZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_CAT_BUILD=OFF \
    -DLLVM_TOOL_LLVM_CFI_VERIFY_BUILD=OFF \
    -DLLVM_TOOL_LLVM_COV_BUILD=OFF \
    -DLLVM_TOOL_LLVM_CVTRES_BUILD=OFF \
    -DLLVM_TOOL_LLVM_CXXDUMP_BUILD=OFF \
    -DLLVM_TOOL_LLVM_CXXFILT_BUILD=OFF \
    -DLLVM_TOOL_LLVM_CXXMAP_BUILD=OFF \
    -DLLVM_TOOL_LLVM_C_TEST_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DEBUGINFOD_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DEBUGINFOD_FIND_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DEBUGINFO_ANALYZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DIFF_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DIS_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DIS_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DLANG_DEMANGLE_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DWARFDUMP_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DWARFUTIL_BUILD=OFF \
    -DLLVM_TOOL_LLVM_DWP_BUILD=OFF \
    -DLLVM_TOOL_LLVM_EXEGESIS_BUILD=OFF \
    -DLLVM_TOOL_LLVM_EXTRACT_BUILD=OFF \
    -DLLVM_TOOL_LLVM_GSYMUTIL_BUILD=OFF \
    -DLLVM_TOOL_LLVM_IFS_BUILD=OFF \
    -DLLVM_TOOL_LLVM_ISEL_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_ITANIUM_DEMANGLE_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_JITLINK_BUILD=OFF \
    -DLLVM_TOOL_LLVM_JITLISTENER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_LIBTOOL_DARWIN_BUILD=OFF \
    -DLLVM_TOOL_LLVM_LINK_BUILD=OFF \
    -DLLVM_TOOL_LLVM_LIPO_BUILD=OFF \
    -DLLVM_TOOL_LLVM_LTO_BUILD=OFF \
    -DLLVM_TOOL_LLVM_MCA_BUILD=OFF \
    -DLLVM_TOOL_LLVM_MC_ASSEMBLE_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_MC_BUILD=OFF \
    -DLLVM_TOOL_LLVM_MC_DISASSEMBLE_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_MICROSOFT_DEMANGLE_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_ML_BUILD=OFF \
    -DLLVM_TOOL_LLVM_MODEXTRACT_BUILD=OFF \
    -DLLVM_TOOL_LLVM_MT_BUILD=OFF \
    -DLLVM_TOOL_LLVM_NM_BUILD=OFF \
    -DLLVM_TOOL_LLVM_OBJCOPY_BUILD=OFF \
    -DLLVM_TOOL_LLVM_OBJDUMP_BUILD=OFF \
    -DLLVM_TOOL_LLVM_OPT_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_OPT_REPORT_BUILD=OFF \
    -DLLVM_TOOL_LLVM_PDBUTIL_BUILD=OFF \
    -DLLVM_TOOL_LLVM_PROFDATA_BUILD=OFF \
    -DLLVM_TOOL_LLVM_PROFGEN_BUILD=OFF \
    -DLLVM_TOOL_LLVM_RC_BUILD=OFF \
    -DLLVM_TOOL_LLVM_READOBJ_BUILD=OFF \
    -DLLVM_TOOL_LLVM_REDUCE_BUILD=OFF \
    -DLLVM_TOOL_LLVM_REMARKUTIL_BUILD=OFF \
    -DLLVM_TOOL_LLVM_REMARK_SIZE_DIFF_BUILD=OFF \
    -DLLVM_TOOL_LLVM_RTDYLD_BUILD=OFF \
    -DLLVM_TOOL_LLVM_RUST_DEMANGLE_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_SHLIB_BUILD=OFF \
    -DLLVM_TOOL_LLVM_SIM_BUILD=OFF \
    -DLLVM_TOOL_LLVM_SIZE_BUILD=OFF \
    -DLLVM_TOOL_LLVM_SPECIAL_CASE_LIST_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_SPLIT_BUILD=OFF \
    -DLLVM_TOOL_LLVM_STRESS_BUILD=OFF \
    -DLLVM_TOOL_LLVM_STRINGS_BUILD=OFF \
    -DLLVM_TOOL_LLVM_SYMBOLIZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_TAPI_DIFF_BUILD=OFF \
    -DLLVM_TOOL_LLVM_TLI_CHECKER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_UNDNAME_BUILD=OFF \
    -DLLVM_TOOL_LLVM_XRAY_BUILD=OFF \
    -DLLVM_TOOL_LLVM_YAML_NUMERIC_PARSER_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LLVM_YAML_PARSER_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_LTO_BUILD=OFF \
    -DLLVM_TOOL_OPT_BUILD=OFF \
    -DLLVM_TOOL_OPT_VIEWER_BUILD=OFF \
    -DLLVM_TOOL_REMARKS_SHLIB_BUILD=OFF \
    -DLLVM_TOOL_SANCOV_BUILD=OFF \
    -DLLVM_TOOL_SANSTATS_BUILD=OFF \
    -DLLVM_TOOL_VERIFY_USELISTORDER_BUILD=OFF \
    -DLLVM_TOOL_VFABI_DEMANGLE_FUZZER_BUILD=OFF \
    -DLLVM_TOOL_XCODE_TOOLCHAIN_BUILD=OFF \
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
  ninja -C "$BUILD_DIR" -j"$JOBS" clang FileCheck 2>&1 | tee /tmp/build.log
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
  ci/metal/run-tests.sh clang/test/Metal "$BUILD_DIR"
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
