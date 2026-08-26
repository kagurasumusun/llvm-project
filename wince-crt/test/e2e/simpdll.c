/* Minimal DLL with exports. */
#include <windows.h>

__declspec(dllexport) int adder (int a, int b) { return a + b; }
__declspec(dllexport) int dllmul (int a, int b) { return a * b; }

BOOL WINAPI DllMain (HANDLE h, DWORD reason, LPVOID res)
{
  (void) h; (void) res;
  switch (reason)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_PROCESS_DETACH:
      break;
    }
  return TRUE;
}
