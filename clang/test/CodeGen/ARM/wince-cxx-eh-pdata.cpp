// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fexceptions -fcxx-exceptions -S -o - %s | FileCheck %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -fexceptions -fcxx-exceptions -c -o /dev/null %s
// RUN: %clang --target=arm-pc-wince -Wno-wince-sysroot-missing -mthumb -fexceptions -fcxx-exceptions -c -o /dev/null %s

// Windows CE C++ (Itanium) exceptions, "Option B": a C++ function that
// throws/catches gets BOTH unwind tables. The ARM EHABI frame
// (.fnstart/.personality __gxx_personality_v0/.handlerdata/.fnend, consumed
// by this toolchain's own libunwind) is unchanged. In addition the WinCFI
// frame now claims a CE exception handler,
//
//   .seh_handler __wince_cxx_frame_handler, %except
//
// which is what makes the compressed .pdata entry carry ExceptionFlag=1
// (ARMWinCOFFStreamer::CEEmitUnwindInfo), so the CE kernel dispatches a
// thrown exception through the in-text PDATA_EH pair
//   { __wince_cxx_frame_handler, &FuncInfoB }
// to the C++ frame handler (libcxxabi), which runs the Itanium search /
// cleanup and resumes the unwind. The 16-byte FuncInfoB
// (magic "FINB" 0x424e4946, version 1, flags, reserved extab_va) is placed
// in .text immediately before the pair, i.e. in the 8 bytes before the
// function start is the pair itself. See <wince_cxx_eh.h> /
// utils/wince/WINEH-ABI-FACTS.md.
//
// The -c runs gate the object level, where a .seh_endproc without a
// preceding .seh_endprologue / a handler that claims ExceptionFlag without
// the pair is a hard error.

extern "C" void might_throw(void);

int cpp_func(int x) {
  try {
    might_throw();
  } catch (...) {
    return -1;
  }
  return x;
}

// A C++ function with a catch: the CE C++ frame handler is claimed
// (ExceptionFlag=1) alongside the unchanged EHABI personality frame, and the
// in-text PDATA_EH pair is emitted in the 8 bytes before the function label.
//
// The pair must exist whenever ExceptionFlag=1 is set (the kernel reads it
// from there), so this is what the emission gate must guarantee.  The
// FuncInfoB (magic "FINB" 0x424e4946 = 1112426822, version 1, flags,
// reserved extab_va) sits immediately before the pair; its label is the
// pair's handler-data word.
// CHECK:      [[FI:.Lce_cxx_funcinfo[0-9]+]]:
// CHECK:      .long 1112426822
// CHECK:      .long __wince_cxx_frame_handler
// CHECK-NEXT: .long [[FI]]
// CHECK-LABEL: _Z8cpp_funci:
// CHECK:      .seh_proc _Z8cpp_funci
// CHECK:      .seh_handler __wince_cxx_frame_handler, %except
// CHECK:      .fnstart
// CHECK:      .personality __gxx_personality_v0
// CHECK:      .handlerdata
// CHECK:      .fnend
// CHECK:      .seh_endproc

// A nested try/catch (calls another C++ function inside the try): it also
// carries the CE C++ frame handler and its own PDATA_EH pair, so an exception
// raised in cpp_func can be unwound through this frame by the kernel and
// caught here.
// CHECK:      [[FI2:.Lce_cxx_funcinfo[0-9]+]]:
// CHECK:      .long 1112426822
// CHECK:      .long __wince_cxx_frame_handler
// CHECK-NEXT: .long [[FI2]]
// CHECK-LABEL: _Z10pass_alongi:
// CHECK:      .seh_proc _Z10pass_alongi
// CHECK:      .seh_handler __wince_cxx_frame_handler, %except
// CHECK:      .personality __gxx_personality_v0

int pass_along(int x) {
  try {
    return cpp_func(x);
  } catch (...) {
    return 42;
  }
}
