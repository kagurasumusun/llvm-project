// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -x c++ -fexceptions -fcxx-exceptions -O1 -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -x c++ -fexceptions -fcxx-exceptions -O1 -c -o %t.obj %s
// RUN: llvm-readobj --relocations %t.obj | FileCheck %s --check-prefix=OBJ

// The catch(T) type-matching contract between this compiler and the WinCE
// personality reader (libc++abi read_target2_value, __WINCE__ branch):
// the TType entries in the .ARM.extab LSDA are plain ABSOLUTE 4-byte
// type_info references (DW_EH_PE_absptr, carried by IMAGE_REL_ARM_ADDR32),
// never TARGET2 offsets.  Break this (e.g. by emitting pcrel/GOT-style
// entries) and every catch (T) silently stops matching on the device.

struct E {};
void might_throw();

void f() {
  try {
    might_throw();
  } catch (E) {
  }
}

// The catch's TType entry must be an absolute reference to E's typeinfo.
// CHECK: .handlerdata
// CHECK: .long {{.*}}_ZTI1E
// CHECK: .fnend

// OBJ: Relocations [
// OBJ: Section {{.*}}.ARM.extab {
// OBJ: 0x{{[0-9A-F]+}} IMAGE_REL_ARM_ADDR32 _ZTI1E
