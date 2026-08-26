// Device test 6: C++ EXE calling into a C DLL (cross-language DLL ABI).
// Build: clang++ --target=arm-pc-wince cpp-interop.cpp simpdll.dll -o cpp-interop.exe
#include <windows.h>
#include <stdio.h>

__declspec(dllimport) int adder (int a, int b);
__declspec(dllimport) int dllmul (int a, int b);

struct Adder {
  int base;
  Adder(int b) : base(b) {}
  int add(int x) { return adder(base, x); }   // C++ -> C DLL call
};

int main() {
  Adder a(10);
  int s = a.add(32);
  int m = dllmul(6, 7);
  printf("cpp-interop: %d %d\n", s, m);
  return (s == 42 && m == 42) ? 0 : 1;
}
