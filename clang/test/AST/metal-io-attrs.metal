// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -ast-dump %s | FileCheck %s

struct VOut {
  int pos [[position]];
  int flatValue [[flat]];
  int userValue [[user(uid)]];
  int pointSize [[point_size]];
};

struct FOut {
  int color0 [[color(0)]];
  int depthValue [[depth(any)]];
};

[[early_fragment_tests]] fragment FOut fragment_io() {
  FOut r;
  return r;
}

// CHECK: MetalPositionAttr
// CHECK: MetalFlatAttr
// CHECK: MetalUserAttr
// CHECK: MetalPointSizeAttr
// CHECK: MetalColorAttr
// CHECK: MetalDepthAttr
// CHECK: MetalEarlyFragmentTestsAttr
