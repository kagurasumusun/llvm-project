# REQUIRES: arm-registered-target
# RUN: split-file %s %t.dir

# RUN: llvm-dlltool -m armce -k -d %t.dir/foo.def -D foo.dll -l %t.dir/foo.lib

# RUN: llvm-mc -triple arm-pc-wince -filetype=obj -o %t.dir/main.obj %t.dir/main.s
# RUN: lld-link /out:%t.dir/main.exe /subsystem:windowsce /entry:entry /base:0x10000 /fixed %t.dir/main.obj %t.dir/foo.lib
# RUN: llvm-objdump -d --no-show-raw-insn %t.dir/main.exe | FileCheck %s --check-prefix=EXE
# RUN: llvm-readobj --coff-basereloc %t.dir/main.exe | FileCheck %s --check-prefix=EXENORELOC
# RUN: llvm-readobj --coff-imports %t.dir/main.exe | FileCheck %s --check-prefix=IMP

# RUN: lld-link /out:%t.dir/main.dll /dll /subsystem:windowsce /entry:entry /base:0x10000000 %t.dir/main.obj %t.dir/foo.lib
# RUN: llvm-readobj --coff-basereloc --sections %t.dir/main.dll | FileCheck %s --check-prefix=DLLRELOC

#--- foo.def
LIBRARY foo.dll
EXPORTS
  func

#--- main.s
	.syntax unified
	.arm
	.text
	.globl	entry
entry:
	bx	lr

	.data
	.globl	fptr
fptr:
	.long	func

# EXE: ldr r12, [pc]
# EXE-NEXT: ldr pc, [r12]

# EXENORELOC: BaseReloc [
# EXENORELOC-NEXT: ]

# IMP: Name: foo.dll
# IMP: func

# DLLRELOC: Name: .reloc
# DLLRELOC: Type: HIGHLOW
