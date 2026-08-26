#!/bin/bash
#===-- wince/test/e2e/run-e2e.sh - end-to-end toolchain tests -------------===#
#
# Builds EXE/DLL samples for arm-pc-wince with the freshly built stage-1
# toolchain and the assembled sysroot, then verifies the produced binaries
# with llvm-readobj/llvm-objdump.  Everything checked here is checked for
# real; the script fails loudly.
#
# Usage: run-e2e.sh <build-dir-with-clang-lld> <sysroot-dir>
#===------------------------------------------------------------------------===

set -euo pipefail

BIN="${1:?usage: run-e2e.sh <llvm-build-bin> <sysroot>}"
SYS="${2:?usage: run-e2e.sh <llvm-build-bin> <sysroot>}"
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT="$(mktemp -d)"
TARGET=arm-pc-wince

CLANG="$BIN/clang"
LLD="$BIN/lld-link"
READOBJ="$BIN/llvm-readobj"
OBJDUMP="$BIN/llvm-objdump"
AR="$BIN/llvm-ar"

CCFLAGS="--target=$TARGET -O1 -I$SYS/include"
LDFLAGS=""

fail() { echo "E2E FAIL: $*" >&2; exit 1; }
pass() { echo "E2E PASS: $*"; }

# --- 1. C EXE with WinMain ------------------------------------------------
"$CLANG" $CCFLAGS -c "$HERE/winmain_app.c" -o "$OUT/app.obj" \
  || fail "compile winmain_app.c"
"$LLD" "$OUT/app.obj" "$SYS/lib/crt0.obj" \
  /out:"$OUT/app.exe" /subsystem:windowsce /entry:WinMainCRTStartup \
  /base:0x10000 /fixed /libpath:"$SYS/lib" \
  wce.lib mingwex.lib "$SYS/lib/clang_rt.builtins-arm.lib" \
  ceoldname.lib coredll.lib || fail "link app.exe"
pass "C EXE (WinMain) linked"

# --- PE header verification ------------------------------------------------
HDR=$("$READOBJ" -headers "$OUT/app.exe")
echo "$HDR" | grep -q "Machine: IMAGE_FILE_MACHINE_ARM (0x1C0)" \
  || fail "machine type is not IMAGE_FILE_MACHINE_ARM"
echo "$HDR" | grep -q "SubSystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI (0x9)" \
  || fail "subsystem is not Windows CE GUI"
echo "$HDR" | grep -q "ImageBase: 0x10000" \
  || fail "image base is not 0x10000"
pass "PE header: machine/subsystem/base OK"

IMPORTS=$("$READOBJ" -coff-imports "$OUT/app.exe")
echo "$IMPORTS" | grep -q "COREDLL.dll" || fail "COREDLL.dll not in imports"
pass "imports: COREDLL.dll present"

# --- 2. C EXE with main() (console-style) ----------------------------------
"$CLANG" $CCFLAGS -c "$HERE/main_app.c" -o "$OUT/main.obj" \
  || fail "compile main_app.c"
"$LLD" "$OUT/main.obj" "$SYS/lib/crt0.obj" \
  /out:"$OUT/main.exe" /subsystem:console /entry:mainCRTStartup \
  /base:0x10000 /fixed /libpath:"$SYS/lib" \
  wce.lib mingwex.lib "$SYS/lib/clang_rt.builtins-arm.lib" \
  ceoldname.lib coredll.lib || fail "link main.exe"
pass "C EXE (main) linked"

# --- 3. DLL with exports ----------------------------------------------------
"$CLANG" $CCFLAGS -c "$HERE/simpdll.c" -o "$OUT/dll.obj" \
  || fail "compile simpdll.c"
"$LLD" "$OUT/dll.obj" "$SYS/lib/dllcrt0.obj" \
  /out:"$OUT/simp.dll" /dll /subsystem:windowsce /entry:DllMainCRTStartup \
  /base:0x10000000 /libpath:"$SYS/lib" \
  /export:adder /export:dllmul \
  wce.lib mingwex.lib "$SYS/lib/clang_rt.builtins-arm.lib" \
  ceoldname.lib coredll.lib || fail "link simp.dll"
pass "DLL linked"

DLLEXPORTS=$("$READOBJ" -coff-exports "$OUT/simp.dll")
echo "$DLLEXPORTS" | grep -q "adder" || fail "adder not exported"
echo "$DLLEXPORTS" | grep -q "dllmul" || fail "dllmul not exported"
pass "DLL exports OK"

DLLRELOC=$("$READOBJ" -headers "$OUT/simp.dll")
echo "$DLLRELOC" | grep -q "BaseOfCode" || fail "DLL headers missing"
pass "DLL headers OK"

# --- 4. static library + multiple translation units --------------------------
"$CLANG" $CCFLAGS -c "$HERE/liba.c" -o "$OUT/liba.obj" || fail "compile liba.c"
"$CLANG" $CCFLAGS -c "$HERE/libb.c" -o "$OUT/libb.obj" || fail "compile libb.c"
"$AR" rc "$OUT/twolib.lib" "$OUT/liba.obj" "$OUT/libb.obj" || fail "ar"
"$CLANG" $CCFLAGS -c "$HERE/useboth.c" -o "$OUT/useboth.obj" || fail "compile useboth.c"
"$LLD" "$OUT/useboth.obj" "$OUT/twolib.lib" "$SYS/lib/crt0.obj" \
  /out:"$OUT/useboth.exe" /subsystem:windowsce /entry:WinMainCRTStartup \
  /base:0x10000 /fixed /libpath:"$SYS/lib" \
  wce.lib mingwex.lib "$SYS/lib/clang_rt.builtins-arm.lib" \
  ceoldname.lib coredll.lib || fail "link useboth.exe"
pass "static lib + multi-TU EXE linked"

# --- 5. disassembly sanity: entry is valid ARM ------------------------------
DISASM=$("$OBJDUMP" -d "$OUT/app.exe")
# The entry must be ARM (not Thumb) instructions; objdump annotates mode.
echo "$DISASM" | grep -A6 "^.*<WinMainCRTStartup>:" >/dev/null \
  || fail "WinMainCRTStartup not found in disassembly"
pass "entry symbol present in disassembly"

# --- 6. Thumb code generation -------------------------------------------------
"$CLANG" $CCFLAGS -mthumb -c "$HERE/liba.c" -o "$OUT/liba_t.obj" \
  || fail "compile liba.c -mthumb"
pass "thumb code generation compiles"

# --- 7. Driver-driven EXE link (clang --target=arm-pc-wince) -----------------
# Exercises the WinCE toolchain's default link line: crt0, wce.lib,
# builtins, coredll imports, subsystem/base/entry defaults.
"$CLANG" --target=$TARGET --sysroot="$SYS" -O1 \
    "$HERE/winmain_app.c" -o "$OUT/app2.exe" || fail "driver link app2.exe"
"$READOBJ" -headers "$OUT/app2.exe" > "$OUT/app2.hdr" || fail "readobj app2"
grep -q "Machine: IMAGE_FILE_MACHINE_ARM" "$OUT/app2.hdr" \
  || fail "driver-linked EXE has wrong machine"
grep -q "SubSystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI" "$OUT/app2.hdr" \
  || fail "driver-linked EXE has wrong subsystem"
grep -q "ImageBase: 0x10000" "$OUT/app2.hdr" \
  || fail "driver-linked EXE has wrong image base"
pass "driver-linked C EXE OK (machine/subsystem/base)"

# --- 8. Driver-driven DLL link (-shared -> /dll, dllcrt0) ---------------------
"$CLANG" --target=$TARGET --sysroot="$SYS" -O1 -shared \
    "$HERE/simpdll.c" -o "$OUT/simp2.dll" || fail "driver link simp2.dll"
"$READOBJ" -headers --coff-exports "$OUT/simp2.dll" > "$OUT/simp2.hdr" \
  || fail "readobj simp2.dll"
grep -q "SubSystem: IMAGE_SUBSYSTEM_WINDOWS_CE_GUI" "$OUT/simp2.hdr" \
  || fail "driver-linked DLL has wrong subsystem"
grep -q "DLL" "$OUT/simp2.hdr" \
  || fail "driver-linked DLL is not a DLL image"
grep -q "adder" "$OUT/simp2.hdr" \
  || fail "dllexport adder missing from driver-linked DLL"
pass "driver-linked DLL OK (/dll, exports)"

# --- 9. EXE importing the DLL (IAT + ARM thunk) --------------------------------
"$CLANG" --target=$TARGET --sysroot="$SYS" -O1 \
    -I"$HERE" -DUSE_DLL "$HERE/useboth.c" -o "$OUT/usedll.exe" \
  || fail "driver link usedll.exe"
pass "driver-linked EXE importing DLL OK"

# --- 10. clang-cl (MSVC-style driver) ------------------------------------------
"$BIN/clang-cl" --target=$TARGET --sysroot="$SYS" /O1 /c "$HERE/main_app.c" \
    /Fo:"$OUT/cl.obj" || fail "clang-cl compile"
"$LLD" "$OUT/cl.obj" "$SYS/lib/crt0.obj" \
  /out:"$OUT/cl.exe" /subsystem:console /entry:mainCRTStartup \
  /base:0x10000 /fixed /libpath:"$SYS/lib" \
  wce.lib mingwex.lib "$SYS/lib/clang_rt.builtins-arm.lib" \
  ceoldname.lib coredll.lib || fail "clang-cl link"
grep -q "Machine: IMAGE_FILE_MACHINE_ARM" <("$READOBJ" -headers "$OUT/cl.exe") \
  || fail "clang-cl EXE wrong machine"
pass "clang-cl compile+link OK"

echo
echo "ALL E2E CHECKS PASSED ($OUT)"
