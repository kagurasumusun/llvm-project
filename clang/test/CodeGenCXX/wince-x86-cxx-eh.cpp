// RUN: %clang_cc1 -emit-llvm -o - -triple=i386-pc-wince -std=c++11 -fcxx-exceptions -fms-extensions %s \
// RUN:     | FileCheck %s
// REQUIRES: x86-registered-target
//
// Windows CE x86 native C++ exception handling.
//
// x86-pc-wince selects the Microsoft C++ ABI (WinCETargetInfo sets
// TargetCXXABI::Microsoft), so a throw lowers to _CxxThrowException with the
// MSVC .xdata ThrowInfo / CatchableType tables, and a function with a
// try/catch gets the __CxxFrameHandler3 personality -- the MSVC-native C++ EH
// that rides the Win32 fs:[0] SEH chain.  coredll exports __CxxFrameHandler3,
// _CxxThrowException and the RTTI helpers (__RTDynamicCast / __RTtypeid /
// __RTCastToVoid), so no SJLJ or DWARF (.eh_frame) unwinder is required and
// no dl_iterate_phdr (absent on CE) is needed.
//
// This is the x86 counterpart of the ARM EHABI path: ARM WinCE uses the
// Itanium/EHABI ABI (WinCEARMTargetInfo -> GenericARM, libc++abi), while x86
// WinCE uses its platform-native Microsoft ABI.  The two never share objects.
// See utils/wince/WINEH-ABI-FACTS.md 4o.

struct X {
  ~X();
};

extern "C" void wince_x86_thrower(const X &x) {
  // CHECK-LABEL: define{{.*}}@wince_x86_thrower(
  // CHECK: call void @_CxxThrowException
  throw x;
}

extern "C" void wince_x86_catcher() {
  // CHECK-LABEL: define{{.*}}@wince_x86_catcher(
  // CHECK: personality ptr @__CxxFrameHandler3
  try {
    throw X();
  } catch (X) {
  }
}
