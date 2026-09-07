


// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fsyntax-only -fms-extensions -Xclang -verify -Wno-unused-variable %s


extern extern int duplicate_extern;      // expected-warning {{duplicate 'extern' declaration specifier}}
extern extern extern int triple_extern;  // expected-warning 2 {{duplicate 'extern' declaration specifier}}
static static int duplicate_static;      // expected-warning {{duplicate 'static' declaration specifier}}


inline extern int reordered_inline(void);
extern inline int reordered_inline2(void);
extern __inline int msvc_style(void);
extern __forceinline int forced(void);

int use(void) {
  return duplicate_extern + triple_extern + duplicate_static;
}
