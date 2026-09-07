// RUN: %clang --target=arm-pc-wince -c %s -o %t.obj
// RUN: llvm-nm %t.obj | FileCheck %s --check-prefix=SYM
// RUN: %clang --target=arm-pc-wince -c %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=DRV


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
