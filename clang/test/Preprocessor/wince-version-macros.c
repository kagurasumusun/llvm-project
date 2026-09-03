// Microsoft spell _WIN32_WCE in "VRR" form: V is the major version and RR is
// the *two-digit decimal* CE "core version" revision, one decimal digit per
// hexadecimal nibble.  Windows CE 4.2 has core version 4.20, so the macro is
// 0x0420 (decimal 1056) -- the value w32api's headers compare against
// (aygshell.h: `#if (_WIN32_WCE >= 0x0420)`, shellapi.h: `>= 0x420`).
// Encoding it as major*0x100 + minor instead produced 0x0402 (decimal 1026)
// for wince4.2, and 0x0402 < 0x0420, so every CE 4.1/4.2-gated API was
// silently disabled.
//
// A single-digit minor is read as the tens digit (wince4.2 == wince4.20); a
// revision with a leading zero (CE 1.01 -> 0x0101) cannot be expressed
// through the triple because VersionTuple drops it, so pass
// -D_WIN32_WCE=0x0101 for those historical builds.
//
// REQUIRES: arm-registered-target, x86-registered-target
//
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince < /dev/null | FileCheck -match-full-lines -check-prefix=DEFAULT %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince7.0 < /dev/null | FileCheck -match-full-lines -check-prefix=CE700 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince6.0 < /dev/null | FileCheck -match-full-lines -check-prefix=CE600 %s
// RUN: %clang_cc1 -E -dM -triple i386-pc-wince5.0 < /dev/null | FileCheck -match-full-lines -check-prefix=CE500 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince4.2 < /dev/null | FileCheck -match-full-lines -check-prefix=CE420 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince4.20 < /dev/null | FileCheck -match-full-lines -check-prefix=CE420 %s
// RUN: %clang_cc1 -E -dM -triple i386-pc-wince4.2 < /dev/null | FileCheck -match-full-lines -check-prefix=CE420 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince4.1 < /dev/null | FileCheck -match-full-lines -check-prefix=CE410 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince4.10 < /dev/null | FileCheck -match-full-lines -check-prefix=CE410 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince4.0 < /dev/null | FileCheck -match-full-lines -check-prefix=CE400 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince3.0 < /dev/null | FileCheck -match-full-lines -check-prefix=CE300 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince2.12 < /dev/null | FileCheck -match-full-lines -check-prefix=CE212 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince2.11 < /dev/null | FileCheck -match-full-lines -check-prefix=CE211 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince2.10 < /dev/null | FileCheck -match-full-lines -check-prefix=CE210 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince2.0 < /dev/null | FileCheck -match-full-lines -check-prefix=CE200 %s
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince1.0 < /dev/null | FileCheck -match-full-lines -check-prefix=CE100 %s
//
// An explicit -D always wins over the versioned triple (CE 1.01).
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince1.0 -D_WIN32_WCE=0x0101 < /dev/null | FileCheck -match-full-lines -check-prefix=CE101 %s

// No version suffix -> default to CE 6.0, the primary deployment target.
// DEFAULT:#define _WIN32_WCE 1536

// 0x0700 = 1792
// CE700:#define _WIN32_WCE 1792
// 0x0600 = 1536
// CE600:#define _WIN32_WCE 1536
// 0x0500 = 1280
// CE500:#define _WIN32_WCE 1280
// 0x0420 = 1056  (NOT 0x0402 = 1026)
// CE420:#define _WIN32_WCE 1056
// 0x0410 = 1040
// CE410:#define _WIN32_WCE 1040
// 0x0400 = 1024
// CE400:#define _WIN32_WCE 1024
// 0x0300 = 768
// CE300:#define _WIN32_WCE 768
// 0x0212 = 530
// CE212:#define _WIN32_WCE 530
// 0x0211 = 529
// CE211:#define _WIN32_WCE 529
// 0x0210 = 528
// CE210:#define _WIN32_WCE 528
// 0x0200 = 512
// CE200:#define _WIN32_WCE 512
// 0x0100 = 256
// CE100:#define _WIN32_WCE 256
// The -D override is echoed verbatim, so CE 1.01 stays reachable.
// CE101:#define _WIN32_WCE 0x0101
