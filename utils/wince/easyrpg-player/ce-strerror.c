/* WinCE mingwrt does not declare strerror/remove. Stub for libpng. */
char *strerror(int errnum) {
  (void)errnum;
  return "error";
}

int remove(const char *path) {
  extern int unlink(const char *);
  return unlink(path);
}
