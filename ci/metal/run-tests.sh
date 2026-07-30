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
FILECHECK="$BUILDABS/bin/FileCheck"

for tool in "$CLANG" "$FILECHECK"; do
  [ -x "$tool" ] || { echo "::error::missing $tool"; exit 2; }
done

pass=0 fail=0
failed=()

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
  for cmd in "${cmds[@]}"; do
    # The two substitutions these tests use. %clang_cc1 has to be expanded
    # before %s, or the path would be rewritten inside it.
    cmd=${cmd//'%clang_cc1'/"$CLANG -cc1 -internal-isystem $BUILDABS/lib/clang/16/include -nostdsysteminc"}
    cmd=${cmd//'%s'/"$test"}
    cmd=${cmd//FileCheck/"$FILECHECK"}

    if ! out=$(eval "$cmd" 2>&1); then
      ok=0
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
    printf '%s\n' "$cmd"
    printf '%s\n' "$out" | head -40
    echo "::endgroup::"
    # One annotation per failing test, carrying its first real diagnostic.
    # GitHub caps how many annotations it keeps, so the test name and the
    # reason go in the same line rather than as separate entries.
    reason=$(printf '%s\n' "$out" \
             | grep -E 'error:' \
             | grep -vE '^\s*(check|label|dag|next|not):' \
             | head -1)
    printf '::error::FAIL %s :: %s\n' "$name" "${reason:0:300}"
  fi
done

echo
echo "Passed: $pass  Failed: $fail"
if [ "$fail" -gt 0 ]; then
  printf 'Failed tests:\n'
  printf '  %s\n' "${failed[@]}"
  # A single annotation listing every failure, so the set is visible even
  # when the per-test ones get truncated.
  printf '::error::%d/%d failed: %s\n' "$fail" "$((pass + fail))" "${failed[*]}"
  exit 1
fi
printf '::notice::all %d tests passed\n' "$pass"
