# w32api (vendored)

Vendored in-tree copy of w32api providing the Windows CE platform headers and the
`libce/*.def` module-definition files from which the system-DLL import
libraries are generated (`llvm-dlltool -m armce`).

* Upstream: https://github.com/kagurasumusun/w32api (branch `wip`),
  the CeGCC w32api with WinCE (`libce`) support.
* Vendored at upstream commit `51de0ad`, **unmodified**.  Local pruning
  vs upstream: `CVS/` metadata and `.cvsignore` files removed.
* Built as-is by `utils/wince/build-wince-sysroot.sh` (import libraries
  through its own Makefile rules; headers installed flat, sharing
  `<sysroot>/include` with mingwrt's, the CeGCC layout).
* License: see `README.w32api` / `CONTRIBUTIONS` upstream terms.
