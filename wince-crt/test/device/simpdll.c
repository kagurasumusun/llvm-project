/* Device test 3: DLL with dllexport functions.
 * Build: clang --target=arm-pc-wince -shared simpdll.c -o simpdll.dll
 */
#include <windows.h>

__declspec (dllexport) int adder (int a, int b) { return a + b; }
__declspec (dllexport) int dllmul (int a, int b) { return a * b; }

BOOL WINAPI
DllMain (HANDLE h, DWORD r, LPVOID p)
{
  (void) h; (void) r; (void) p;
  return TRUE;
}
