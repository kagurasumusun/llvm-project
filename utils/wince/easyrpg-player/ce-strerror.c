/* WinCE mingwrt does not declare strerror. Stub for libpng/liblcf. */
char *strerror(int errnum) {
  (void)errnum;
  return "error";
}
