// REQUIRES: arm-registered-target, x86-registered-target
// RUN: %clang -target arm-unknown-wince -fms-extensions -fms-compatibility -S -emit-llvm %s -o - | FileCheck %s
// RUN: %clang -target i386-unknown-wince -fms-extensions -fms-compatibility -S -emit-llvm %s -o - | FileCheck %s
// RUN: %clang -target arm-unknown-wince -fms-extensions -fms-compatibility -c %s -o %t.arm.obj
// RUN: %clang -target i386-unknown-wince -fms-extensions -fms-compatibility -c %s -o %t.x86.obj
// RUN: %clang -target arm-unknown-wince -fms-extensions -fms-compatibility -gcodeview -c %s -o %t.arm-debug.obj
// RUN: %clang -target i386-unknown-wince -fms-extensions -fms-compatibility -gcodeview -c %s -o %t.x86-debug.obj

typedef struct { int value; } Record;
Record global;
struct { int value; } anonymous;

__declspec(dllexport) int __cdecl read_record(Record *record) {
  return record->value + global.value + anonymous.value;
}

// CHECK: @global =
// CHECK: @anonymous =
// CHECK: define{{.*}} @read_record(
