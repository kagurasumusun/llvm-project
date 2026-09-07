// REQUIRES: arm-registered-target, x86-registered-target
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
// RUN: %clang_cc1 -E -dM -triple arm-pc-wince1.0 -D_WIN32_WCE=0x0101 < /dev/null | FileCheck -match-full-lines -check-prefix=CE101 %s

// DEFAULT:#define _WIN32_WCE 1536

// CE700:#define _WIN32_WCE 1792
// CE600:#define _WIN32_WCE 1536
// CE500:#define _WIN32_WCE 1280
// CE420:#define _WIN32_WCE 1056
// CE410:#define _WIN32_WCE 1040
// CE400:#define _WIN32_WCE 1024
// CE300:#define _WIN32_WCE 768
// CE212:#define _WIN32_WCE 530
// CE211:#define _WIN32_WCE 529
// CE210:#define _WIN32_WCE 528
// CE200:#define _WIN32_WCE 512
// CE100:#define _WIN32_WCE 256
// CE101:#define _WIN32_WCE 0x0101
