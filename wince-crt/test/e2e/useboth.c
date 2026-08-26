#include <windows.h>
extern int liba_add (int, int);
extern int libb_mul (int, int);

int PASCAL WinMain (HINSTANCE h, HINSTANCE p, LPWSTR c, int s)
{
  if (liba_add (2, 3) != 5)
    return 1;
  if (libb_mul (2, 3) != 6)
    return 2;
  return 0;
}
