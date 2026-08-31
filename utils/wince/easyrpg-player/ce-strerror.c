/* WinCE mingwrt/coredll gaps used by libpng and pixman. Do not patch those trees. */
#include <windows.h>

char *strerror(int errnum) {
  (void)errnum;
  return "error";
}

int remove(const char *path) {
  extern int unlink(const char *);
  return unlink(path);
}

HANDLE WINAPI CreateMutexA(LPSECURITY_ATTRIBUTES attr, BOOL initial, LPCSTR name) {
  wchar_t wbuf[260];
  LPCWSTR wname = NULL;
  if (name) {
    MultiByteToWideChar(CP_ACP, 0, name, -1, wbuf, 260);
    wname = wbuf;
  }
  return CreateMutexW(attr, initial, wname);
}
