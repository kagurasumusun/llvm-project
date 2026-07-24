// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only -verify %s

// Metal function constants live in the constant address space but are supplied
// by specialization data, so an initializer is not required.
constant bool fc_bool [[function_constant(0)]];
constant int fc_int [[function_constant(1)]];

// Keep the generic constant-address-space rule for ordinary non-extern globals.
constant int missing_initializer; // expected-error {{variable in constant address space must be initialized}}
