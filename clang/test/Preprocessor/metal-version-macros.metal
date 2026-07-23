// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -E -dM %s | FileCheck %s --check-prefix=METAL40
// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4 -E -dM %s | FileCheck %s --check-prefix=METAL40
// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -E -dM %s | FileCheck %s --check-prefix=DEFAULT

// METAL40-DAG: #define __METAL__ 1
// METAL40-DAG: #define __METAL_MACOS__ 1
// METAL40-DAG: #define __METAL_VERSION__ 400
// DEFAULT-DAG: #define __METAL_VERSION__ 400
