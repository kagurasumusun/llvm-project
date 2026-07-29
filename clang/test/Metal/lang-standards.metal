// Verify the C++ language base of each MSL version.
//
// The expected values are measurements of Apple's compiler recorded in
// research/spec/METAL_CXX_GENERATIONS_MAP.md and METAL_CXX_MASTER_ATLAS.md,
// taken with `metalfe -E -dM`.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=macos-metal1.0 -E -dM %s | FileCheck --check-prefix=CXX11 %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=macos-metal1.2 -E -dM %s | FileCheck --check-prefix=CXX11 %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=macos-metal2.0 -E -dM %s | FileCheck --check-prefix=CXX14 %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=macos-metal2.4 -E -dM %s | FileCheck --check-prefix=CXX14 %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.0 -E -dM %s | FileCheck --check-prefix=CXX14 %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -E -dM %s | FileCheck --check-prefix=CXX14 %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal4.0 -E -dM %s | FileCheck --check-prefix=CXX17 %s
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal4.1 -E -dM %s | FileCheck --check-prefix=CXX17 %s

// CXX11: #define __cplusplus 201103L
// CXX14: #define __cplusplus 201402L
// CXX17: #define __cplusplus 201703L
