/// Full-syntax MSVC pragmas (Windows CE / eMbedded Visual C++ era sources).
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fsyntax-only -fms-extensions -Xclang -verify %s

#pragma auto_inline(off)
int not_inlined_1(void) { return 1; }
#pragma auto_inline(on)
int maybe_inlined(void) { return 2; }

#pragma auto_inline()   // empty parens == on
#pragma auto_inline(on)
#pragma check_stack(off)
int no_probes_1(void) { return 3; }
#pragma check_stack()
#pragma check_stack(on)

#pragma setlocale("japanese")   // expected-warning {{'#pragma setlocale(japanese)'}}
#pragma setlocale("C")          // expected-warning {{'#pragma setlocale(C)'}}
#pragma conform(name, on)       // expected-warning {{'#pragma conform(on)'}}
#pragma conform(name, off, push, forScope) // expected-warning {{'#pragma conform(off)'}}
#pragma conform(name, on, pop)  // expected-warning {{'#pragma conform(on)'}}

// Malformed uses are diagnosed against the MSVC grammar.
#pragma auto_inline(energy)     // expected-warning {{unexpected argument 'energy' to '#pragma auto_inline'}}
#pragma check_stack(energy)     // expected-warning {{unexpected argument 'energy' to '#pragma check_stack'}}
#pragma setlocale(japanese)     // expected-warning {{expected string literal in '#pragma setlocale'}}
#pragma conform(scope, on)      // expected-warning {{unexpected argument 'scope' to '#pragma conform'}}

int f(void) { return 0; }
