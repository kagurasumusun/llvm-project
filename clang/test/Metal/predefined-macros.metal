// The Metal specific predefined macros.
//
// Expected values transcribed from a measurement of metalfe-32023.883 in
// reference/metal-ast-macos-air64/meta/metal-predefined-macros.txt and the
// per-standard dumps in reference/metal-ast-macos-air64/driver/*.macros.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal4.0 -E -dM %s | FileCheck --check-prefix=METAL40 %s
// RUN: %clang_cc1 -x metal -triple air64_v20-apple-macosx10.13.0 \
// RUN:   -std=macos-metal1.1 -E -dM %s | FileCheck --check-prefix=METAL11 %s

// MSL 3.0 and later identify themselves with __METAL__ and no __METAL_MACOS__.
// METAL40-DAG: #define __AIR64__ 1
// METAL40-DAG: #define __AIR_VERSION__ 20800
// METAL40-DAG: #define __METAL_VERSION__ 400
// METAL40-DAG: #define __METAL__ 1
// METAL40-DAG: #define __METAL_ACCESS_READ_WRITE__ 3
// METAL40-DAG: #define __METAL_MEMORY_SCOPE_THREADGROUP__ 1
// METAL40-NOT: #define __METAL_MACOS__

// The versioned standards do the opposite: __METAL_MACOS__ and no __METAL__.
// This distinction is what <metal_config> keys its __HAVE_* capability macros
// off, so getting it wrong silently disables every capability macro.
// METAL11-DAG: #define __AIR_VERSION__ 20000
// METAL11-DAG: #define __METAL_MACOS__ 1
// METAL11-DAG: #define __METAL_VERSION__ 110
// METAL11-NOT: #define __METAL__ 1
