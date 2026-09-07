// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -x c++ -fexceptions -fcxx-exceptions -O1 -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -x c++ -fexceptions -fcxx-exceptions -O1 -c -o %t.obj %s
// RUN: llvm-readobj --relocations %t.obj | FileCheck %s --check-prefix=OBJ


struct E {};
void might_throw();

void f() {
  try {
    might_throw();
  } catch (E) {
  }
}

// CHECK: .handlerdata
// CHECK: .long {{.*}}_ZTI1E
// CHECK: .fnend

// OBJ: Relocations [
// OBJ: Section {{.*}}.ARM.extab {
// OBJ: 0x{{[0-9A-F]+}} IMAGE_REL_ARM_ADDR32 _ZTI1E
