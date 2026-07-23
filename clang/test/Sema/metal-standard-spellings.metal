// RUN: %clang_cc1 -triple air64-apple-macosx10.15 -x metal -std=macos-metal2.0 -fsyntax-only %s
// RUN: %clang_cc1 -triple air64-apple-macosx10.15 -x metal -std=osx-metal2.0 -fsyntax-only %s
// RUN: %clang_cc1 -triple air64-apple-ios10.3 -x metal -std=ios-metal1.0 -fsyntax-only %s
// RUN: not %clang_cc1 -triple air64-apple-macosx10.15 -x metal -std=ios-metal2.0 -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix=IOS-ON-MAC
// RUN: not %clang_cc1 -triple air64-apple-ios10.3 -x metal -std=macos-metal1.1 -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix=MAC-ON-IOS
// RUN: not %clang_cc1 -triple air64-apple-macosx10.15 -x metal -std=metal2.0 -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix=OLD-UNQUALIFIED

// IOS-ON-MAC: invalid argument '-std=ios-metal2.0' not allowed with
// MAC-ON-IOS: invalid argument '-std=macos-metal1.1' not allowed with
// OLD-UNQUALIFIED: invalid value 'metal2.0' in '-std=metal2.0'

kernel void k(device int *out [[buffer(0)]]) { out[0] = 0; }
