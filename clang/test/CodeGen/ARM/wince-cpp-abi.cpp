// RUN: %clang --target=arm-pc-wince -c %s -o %t.obj
// RUN: llvm-nm %t.obj | FileCheck %s --check-prefix=SYM
// RUN: %clang --target=arm-pc-wince -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DRV

// Pins the CE C++ ABI choices the driver enforces, so a silent ABI
// drift is a lit failure:
//  * Itanium name mangling (NOT MSVC mangling),
//  * 16-bit unsigned wchar_t, 32-bit pointers/longs (compile-time
//    size probes),
//  * the MS-compat flags the driver always passes for w32api
//    interop.

// DRV: "-fwchar-type=short"
// DRV: "-fno-signed-wchar"
// DRV: "-fms-extensions"

typedef char probe_ptr[sizeof(void *) == 4 ? 1 : -1];
typedef char probe_long[sizeof(long) == 4 ? 1 : -1];
typedef char probe_wchar[sizeof(wchar_t) == 2 ? 1 : -1];
typedef char probe_bool[sizeof(bool) == 1 ? 1 : -1];

int add(int a, int b) { return a + b; }
wchar_t wcprobe(void) { return 0; }

// SYM: _Z3addii
// SYM: _Z7wcprobev
