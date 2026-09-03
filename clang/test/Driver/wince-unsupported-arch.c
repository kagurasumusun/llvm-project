/// Windows CE ran on five architectures: SH3/SH4 (CE 1.0-5.0), MIPS
/// (CE 1.0-5.0), PowerPC (CE 2.x only), ARM (CE 2.0-6.0) and x86 (CE 1.0-6.0).
/// This driver accepts only ARM and x86, and the allow-list is not arbitrary:
/// it is exactly the set of architectures for which Targets.cpp builds a
/// WinCE-aware TargetInfo - WinCEARMTargetInfo for arm/thumb (the WinCE case
/// in the arm/thumb switch) and WinCETargetInfo for 32-bit x86 (the WinCE case
/// in the x86 switch).  Those are the only two that call addWinCEDefines, so
/// for any other architecture _WIN32_WCE, UNDER_CE, WINCE, __COREDLL__,
/// UNICODE and _UNICODE are silently absent while the triple still claims
/// WinCE, and w32api's "#ifdef _WIN32_WCE" guards take the desktop path.
///
/// The driver picks this toolchain from the OS part of the triple alone
/// (Driver.cpp: case Triple::WinCE), and isOSWindows() is true for WinCE, so a
/// foreign-architecture *-pc-wince triple really does reach WinCE::WinCE.
/// Before the gate it went on to Linker::ConstructJob, whose library choice is
/// the binary test "x86 ? libcoredll6-x86.a : libcoredll6.a" - handing ARM
/// import libraries and libclang_rt.builtins-arm.a to a non-ARM image.  That
/// is not a link error, it is a silently wrong binary, so the architecture is
/// rejected in the constructor instead.  Recording an Error there is enough to
/// stop the build: ExecuteCompilation returns 1 before executing any job, so
/// no compile or link ever runs for a rejected triple.
///
/// MIPS, PowerPC, AArch64 and SuperH are deliberately not named here.
/// WinCE.cmake sets LLVM_TARGETS_TO_BUILD to "ARM;X86", so those backends are
/// not registered in this configuration; SH3/SH4 are unreachable from LLVM at
/// any CE version because LLVM has no SuperH backend.  armeb and x86_64 are
/// the reachable instances of the same defect - both sit inside the ARM and X86
/// backends, and neither is a CE architecture that ever shipped (CE ARM was
/// little-endian ARMV4I; CE x86 was 32-bit only).
// REQUIRES: arm-registered-target, x86-registered-target

/// Big-endian ARM: the armeb/thumbeb switch in Targets.cpp has no WinCE case,
/// so this gets the generic big-endian ARM TargetInfo, not WinCEARMTargetInfo.
// RUN: not %clang -target armeb-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=ARMEB
// ARMEB: error: unsupported architecture 'armeb' for Windows CE target

/// 64-bit x86: x86_64 has its own switch with no WinCE case, so this gets
/// X86_64TargetInfo and none of the WinCE predefines.
// RUN: not %clang -target x86_64-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=X8664
// X8664: error: unsupported architecture 'x86_64' for Windows CE target

/// The gate must not over-reject the two architectures it exists to serve, and
/// each must still get its own COREDLL import library and compiler-rt archive.
// RUN: %clang -target arm-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=OKARM
// OKARM-NOT: unsupported architecture
// OKARM: lld-link
// OKARM: libclang_rt.builtins-arm.a
// OKARM: libcoredll6.a

// RUN: %clang -target i386-pc-wince %s -o /dev/null -### 2>&1 \
// RUN:   | FileCheck %s --check-prefix=OKX86
// OKX86-NOT: unsupported architecture
// OKX86: lld-link
// OKX86: libclang_rt.builtins-i386.a
// OKX86: libcoredll6-x86.a
