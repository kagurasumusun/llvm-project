/* Device test 1: C, WinMain entry, coredll message box.
 * Build: clang --target=arm-pc-wince app-winmain.c -o app-winmain.exe
 * Pass:  a message box titled "clang-wince" appears; closing it exits 0.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int PASCAL
WinMain (HINSTANCE hInst, HINSTANCE hPrev, LPWSTR cmd, int show)
{
  MSGBOXPARAMSW m;
  ZeroMemory (&m, sizeof (m));
  m.cbSize = sizeof (m);
  m.hInstance = hInst;
  m.lpszText = L"hello from LLVM/Clang on Windows CE";
  m.lpszCaption = L"clang-wince";
  m.dwStyle = MB_OK | MB_ICONINFORMATION;
  return MessageBoxIndirectW (&m);
}
