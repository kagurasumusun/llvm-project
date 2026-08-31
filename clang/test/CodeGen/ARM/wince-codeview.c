// RUN: %clang --target=arm-pc-wince -gcodeview -O0 -c %s -o %t.obj
// RUN: llvm-objdump -h %t.obj | FileCheck %s --check-prefix=SECS
// RUN: llvm-objdump -s --section=.debug$S %t.obj | FileCheck %s --check-prefix=LINES

// WinCE accepts CodeView debug info: the object carries the CodeView
// sections.  In LLVM's CodeView layout the line-number table is NOT a
// separate .debug$B section: it is a Lines subsection (kind 0x2) inside
// .debug$S, which is exactly where lld's findLineTable reads it from.
// lld-link -debug (added by the driver with -g) merges these sections
// into the final image; lld has no PDB writer.

int add(int a, int b) { return a + b; }
int main(void) { return add(1, 2) - 3; }

// SECS: .debug$S
// SECS: .debug$T

// The .debug$S hex dump must contain the Lines subsection header word
// (uint32 kind = 2, little-endian 02000000) for main()/add().
// LINES: 02000000
