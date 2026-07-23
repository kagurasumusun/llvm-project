// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only -verify %s

struct GoodColor { int c [[color(7)]]; };
struct BadColor { int c [[color(8)]]; }; // expected-error {{'color' attribute requires integer constant between 0 and 7 inclusive}}
