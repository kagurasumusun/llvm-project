// REQUIRES: arm-registered-target
// RUN: %clang --target=armv7-pc-windows-gnu -ffreestanding -Os -S -emit-llvm %s -o - | FileCheck %s --check-prefix=IR --implicit-check-not=uwtable
// RUN: %clang --target=armv7-pc-windows-gnu -ffreestanding -Os -S %s -o - | FileCheck %s --check-prefix=ASM --implicit-check-not=.seh_
// RUN: %clang --target=armv7-pc-windows-gnu -ffreestanding -Os -c %s -o %t.obj
// RUN: %clang --target=armv7-pc-windows-gnu -ffreestanding -Os -fasynchronous-unwind-tables -S %s -o - | FileCheck %s --check-prefix=SEH
// RUN: %clang --target=armv7-pc-windows-gnu -ffreestanding -Os -fasynchronous-unwind-tables -c %s -o %t.unwind.obj

int no_unwind(int x) {
  volatile int slot = x;
  return slot + 1;
}

// IR-LABEL: define{{.*}} @no_unwind(
// IR: attributes #{{[0-9]+}} = { {{.*}}nounwind{{.*}}optsize

// ASM-LABEL: no_unwind:
// ASM: str
// ASM: ldr

// SEH-LABEL: no_unwind:
// SEH: .seh_proc no_unwind
// SEH: .seh_endprologue
// SEH: .seh_endproc
