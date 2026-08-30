/// MSVC-tolerated `extern`-family storage-class constructs.
/// MSVC accepts redundant identical storage class specifiers
/// (`extern extern int x;` appears in machine-generated and ported
/// headers); clang accepts them as an extension warning, and the
/// reordered spellings behave identically.

// RUN: %clang_cc1 -triple arm-pc-wince -fsyntax-only -fms-extensions -verify -Wno-unused-variable %s

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
