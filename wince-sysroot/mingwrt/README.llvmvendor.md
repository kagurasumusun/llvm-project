# mingwrt (vendored)

Vendored in-tree copy of the mingw-runtime ("mingwrt") used as the C runtime for
the Windows CE target.  Built **as-is** through its own configure/make by
`utils/wince/build-wince-sysroot.sh`; it is not wrapped or re-implemented.

* Upstream: https://github.com/kagurasumusun/mingwrt (branch `master`),
  the CeGCC-lineage mingw-runtime with active WinCE/mingw32ce support.
* Vendored at upstream commit `7c35691` plus one local commit,
  `Build with LLVM/Clang in addition to GCC` (`5ed3cc4` locally):
  * `include/_mingw.h`: the `#ifdef __declspec` probe for
    `__DECLSPEC_SUPPORTED` / `__MINGW_IMPORT` / `_CRTIMP` also accepts
    `__clang__` (GCC's PE targets predefine `__declspec` as a macro;
    Clang implements it as a keyword, so the probe used to fail),
  * `Makefile.in`: the generated `.def` files are preprocessed without
    `-C` (the preserved C comments are outside the def-file grammar
    llvm-dlltool implements; GCC builds are unaffected).
  These changes are compiler-compat only; runtime semantics are upstream's.
* Local pruning vs upstream: `CVS/` metadata and `.cvsignore` files
  removed.  Everything else is verbatim.
* License: see `DISCLAIMER` and `CONTRIBUTORS` (historic mingw.org
  runtime terms; the WinCE additions are CeGCC-licensed).
