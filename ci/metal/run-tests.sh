#!/usr/bin/env bash
#
# Run clang/test/Metal without lit.
#
# lit is a large dependency for what these tests actually use. Every RUN line
# in clang/test/Metal is of the form
#
#     %clang_cc1 <args> %s [| FileCheck <args> %s]
#
# and the only lit substitutions that appear anywhere are %clang_cc1 and %s.
# There is no REQUIRES, UNSUPPORTED or XFAIL, and -verify is handled by clang
# itself, not by lit. So the whole harness is: expand two strings, join
# continuation lines, run the command.
#
# Dropping lit means LLVM_INCLUDE_TESTS can be turned off, which is what
# unblocks the CLANG_TOOL_*/LLVM_TOOL_* switches -- with tests on, check-llvm
# and check-clang list every side tool as a hard dependency and configure
# refuses to drop any of them.
#
# Usage: ci/metal/run-tests.sh [test-dir] [build-dir]
#
set -uo pipefail

TESTDIR=${1:-clang/test/Metal}
BUILD=${2:-${BUILD_DIR:-build}}

# Accept both a relative build dir and an absolute one.
case "$BUILD" in
  /*) BUILDABS="$BUILD" ;;
  *)  BUILDABS="$PWD/$BUILD" ;;
esac
CLANG="$BUILDABS/bin/clang"

# If the Apple standard library was fetched, add its include path so that tests
# can #include <metal_stdlib> and use half, texture2d, sampler etc.
STDLIB_INC=""
if [ -f /tmp/metal_stdlib_path ]; then
  sdk=$(cat /tmp/metal_stdlib_path)
  if [ -n "$sdk" ] && [ -d "$sdk" ]; then
    STDLIB_INC="-I $sdk"
  fi
fi
FILECHECK="$BUILDABS/bin/FileCheck"

for tool in "$CLANG" "$FILECHECK"; do
  [ -x "$tool" ] || { echo "::error::missing $tool"; exit 2; }
done

# Point clang at a symbolizer so a crash prints function names instead of the
# useless "Stack dump without symbol names" address list. The compiler keeps
# its symbol table (Release build with -g0, not stripped), which is all the
# symbolizer needs; frames are demangled with c++filt when annotated below.
if [ -z "${LLVM_SYMBOLIZER_PATH:-}" ]; then
  for sym in $(command -v llvm-symbolizer 2>/dev/null) \
             /usr/bin/llvm-symbolizer-* \
             /usr/lib/llvm-*/bin/llvm-symbolizer; do
    [ -x "$sym" ] && { export LLVM_SYMBOLIZER_PATH="$sym"; break; }
  done
fi
echo "symbolizer: ${LLVM_SYMBOLIZER_PATH:-none found}"

pass=0 fail=0
failed=()
notes=()
probe_notes=()

for test in "$TESTDIR"/*.metal; do
  [ -e "$test" ] || continue
  name=$(basename "$test")

  # Join RUN continuations: a line ending in a backslash continues onto the
  # next RUN line, which is how the multi-line invocations in these files are
  # written.
  mapfile -t cmds < <(
    sed -n 's|^// *RUN: *||p' "$test" \
    | awk '{
        if (cont) { line = line " " $0 } else { line = $0 }
        if (line ~ /\\$/) { sub(/\\$/, "", line); cont = 1 }
        else { print line; cont = 0; line = "" }
      }'
  )

  [ ${#cmds[@]} -gt 0 ] || continue

  ok=1
  out=""
  failed_cmd=""
  for cmd in "${cmds[@]}"; do
    # The two substitutions these tests use. %clang_cc1 has to be expanded
    # before %s, or the path would be rewritten inside it.
    cmd=${cmd//'%clang_cc1'/"$CLANG -cc1 -internal-isystem $BUILDABS/lib/clang/16/include $STDLIB_INC -nostdsysteminc"}
    cmd=${cmd//'%s'/"$test"}
    cmd=${cmd//FileCheck/"$FILECHECK"}

    if ! out=$(eval "$cmd" 2>&1); then
      ok=0
      failed_cmd=$cmd
      # A pipeline into FileCheck hides why the compiler produced nothing:
      # all that survives is "'<stdin>' is empty". Re-run the part before the
      # first pipe to get the compiler's own diagnostics.
      if [ "${cmd#*|}" != "$cmd" ]; then
        compile_only=${cmd%%|*}
        out="$out
--- compiler output alone ---
$(eval "$compile_only" 2>&1 | head -25)"
      fi
      break
    fi
  done

  if [ "$ok" -eq 1 ]; then
    pass=$((pass + 1))
    echo "PASS: $name"
  else
    fail=$((fail + 1))
    failed+=("$name")
    echo "FAIL: $name"
    echo "::group::$name"
    printf '%s\n' "$failed_cmd"
    printf '%s\n' "$out" | head -40
    echo "::endgroup::"
    # One annotation per failing test, carrying its first real diagnostics.
    # GitHub caps how many annotations it keeps, so the test name and the
    # reasons go in the same line rather than as separate entries. Prefer the
    # compiler's own diagnostics over the runner's wrapper output, and squeeze
    # several lines into one annotation (joined with ' < ') so the callee
    # subtree / FileCheck context survives the annotation cap.
    reason=$(printf '%s\n' "$out" \
             | grep -vE '^MBE:|^GCRT:' | head -6 \
             | awk 'BEGIN { ORS="" } { if (NR > 1) printf " < "; printf "%s", $0 }')
    if [ -z "$reason" ]; then
      reason=$(printf '%s\n' "$out" | grep -E 'error:|Assertion' \
               | awk 'BEGIN { ORS="" } { if (NR > 1) printf " < "; printf "%s", $0 }')
    fi
    if [ -z "$reason" ]; then
      reason=$(printf '%s\n' "$out" | head -1)
    fi
    # A crash is re-run under gdb to get a trustworthy backtrace: clang's own
    # stack printer stalls inside the symbolizer protocol after one frame on
    # this binary, but gdb reads the symbol table directly. The frame list is
    # compressed into a single annotation -- addresses, namespaces and
    # argument lists dropped, frames joined with '<', innermost first --
    # because GitHub cuts annotations off after a handful, and the raw log
    # cannot be fetched from some environments (Azure Blob EOFs). Annotations
    # are buffered and flushed after the run summary, which therefore always
    # makes the cut.
    if printf '%s\n' "$out" | grep -qE 'Stack dump|PLEASE submit a bug report'; then
      # The last MBE: checkpoint emitted before the crash pinpoints the
      # failing statement far better than an address backtrace; accompany it
      # with the two tail lines of the output for context. GitHub keeps only
      # a handful of error annotations per step, so this is one annotation.
      frames=$(printf '%s\n' "$out" \
               | grep -E 'MBE:|GCRT:' \
               | tail -8 \
               | awk 'BEGIN { ORS="" } { if (NR > 1) printf " < "; printf "%s", $0 }')
      if [ -z "$frames" ]; then
        # No MBE checkpoints were logged: crash happened before the first
        # instrumented site. The numbered pretty-stack entries at the head of
        # the dump say what clang was doing; take them plus the final lines.
        frames=$( { printf '%s\n' "$out" | grep -E '^[0-9]+\.' | grep -v 'Program arguments'; \
                    printf '%s\n' "$out" | grep -vE '^\s*$' | tail -4; } \
                 | awk 'BEGIN { ORS="" } { if (NR > 1) printf " < "; printf "%s", $0 }' \
                 | cut -c1-280)
      fi
      # The zzbisect probes are the controlled experiments; their notes go
      # first, ahead of the full-suite noise, within GitHub's annotation cap.
      if [[ $name == zzbisect-* ]]; then
        probe_notes+=("::error::CRASH $name :: ${frames:0:400}")
      else
        notes+=("::error::CRASH $name :: ${frames:0:400}")
      fi
    else
      notes+=("::error::FAIL $name :: ${reason:0:500}")
    fi
    # "FileCheck error: '<stdin>' is empty" says nothing about why the
    # compiler produced no output, so surface the compiler's own diagnostics
    # as annotations too -- the raw log cannot be fetched from here.
    while IFS= read -r l; do
      notes+=("::error::  $name: ${l:0:280}")
    done < <(printf '%s\n' "$out" \
             | sed -n '/--- compiler output alone ---/,$p' \
             | grep -E 'error:|warning:|fatal' | head -3)
  fi
done

echo
echo "Passed: $pass  Failed: $fail"
if [ "$fail" -gt 0 ]; then
  printf 'Failed tests:\n'
  printf '  %s\n' "${failed[@]}"
  # The summary annotation goes FIRST: GitHub cuts annotations off after a
  # handful and this line is the only place the full failure set appears.
  printf '::error::%d/%d failed: %s\n' "$fail" "$((pass + fail))" "${failed[*]}"
  printf '%s\n' "${probe_notes[@]}"
  printf '%s\n' "${notes[@]}"
  exit 1
fi
printf '::notice::all %d tests passed\n' "$pass"
