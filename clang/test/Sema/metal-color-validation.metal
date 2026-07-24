// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only -verify %s

struct GoodColor { int c [[color(7)]]; };
struct BadColor { int c [[color(8)]]; }; // expected-error {{'color' attribute requires integer constant between 0 and 7 inclusive}}

struct DuplicateColorOut {
  int c0 [[color(0)]];
  int c1 [[color(0)]]; // expected-error {{'color' attribute index 0 is already used by another stage output field}}
};
fragment DuplicateColorOut duplicate_color_output() {
  DuplicateColorOut r;
  return r;
}

struct DuplicateDepthOut {
  int d0 [[depth(any)]];
  int d1 [[depth(greater)]]; // expected-error {{'depth' attribute is already used by another stage output field}}
};
fragment DuplicateDepthOut duplicate_depth_output() {
  DuplicateDepthOut r;
  return r;
}
