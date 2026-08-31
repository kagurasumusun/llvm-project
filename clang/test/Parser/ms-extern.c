/// MSVC-tolerated `extern`-family storage-class constructs.
/// Under Parser/ because Sema lit substitutions reject %clang, and
/// %clang_cc1 without the driver's WinCE defaults currently SIGSEGVs.
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fsyntax-only -fms-extensions -Xclang -verify -Wno-unused-variable %s


extern extern int duplicate_extern;      // expected-warning {{duplicate 'extern' declaration specifier}}
extern extern extern int triple_extern;  // expected-warning 2 {{duplicate 'extern' declaration specifier}}
static static int duplicate_static;      // expected-warning {{duplicate 'static' declaration specifier}}

// Reordered spellings are the same specifier.
inline extern int reordered_inline(void);
extern inline int reordered_inline2(void);
extern __inline int msvc_style(void);   // gnu89 semantics on the WinCE target
extern __forceinline int forced(void);

int use(void) {
  return duplicate_extern + triple_extern + duplicate_static;
}

// Mixing *different* storage classes is an error (MSVC C2159).  Do not
// exercise `static extern` here: clang currently crashes (-11) on that
// combination for this target, which is a separate Sema bug.
