// `#pragma METAL fp math_mode(...)` is how Apple's <metal_math> flips the
// fast math relaxations for a region of code: reference-apple/clang/
// 32023.883/include/metal uses math_mode(safe) 90 times around bit-level
// helpers that must not be reassociated.  The pragma used to be parsed and
// then dropped on the floor; now it drives the same FPOptions machinery as
// #pragma float_control:
//
//   safe / precise -> FPOptionsOverride::setFPPreciseEnabled(true)
//                     (every relaxation off, contraction included; the
//                      multiplication prints plain `fmul float`, exactly as
//                      measured for #pragma float_control(precise, on) on the
//                      same cc1)
//   fast           -> FPOptionsOverride::setFPPreciseEnabled(false)
//                     (the whole fast-math set, printed as `fast`)
//
// The measured baseline is fast: Apple's default cc1 line carries the
// fast-math flag set and the golden IR shows `fmul fast float`
// (research/golden/P01/metal32_macosx26/probe.ll).  The driver seeds that
// line, so the direct cc1 invocations below pass -ffast-math explicitly to
// stand in for it; the second RUN omits it entirely to prove the pragma
// itself is what toggles the flags.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -ffast-math -emit-llvm -no-opaque-pointers -o - %s | \
// RUN:   FileCheck %s --check-prefix=FM
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -emit-llvm -no-opaque-pointers -o - %s | \
// RUN:   FileCheck %s --check-prefix=STRICT

typedef unsigned int uint;

// Function-local use, the way <metal_math> uses it.  The pragma reaches to
// the end of the enclosing compound statement: ParseCompoundStatementBody
// wraps every body in Sema::FPFeaturesStateRAII, which restores the FP
// options when the closing '}' is consumed.
kernel void local(device float *f [[buffer(0)]],
                  uint i [[thread_position_in_grid]]) {
  float a = f[0] * f[1];
  // FM: fmul fast float
  // STRICT: fmul contract float
#pragma METAL fp math_mode(safe)
  float b = f[2] * f[3];
  // FM: fmul float
  // STRICT: fmul float
  {
#pragma METAL fp math_mode(fast)
    // An inner scope may raise the mode again.
    float c = b * b;
    // FM: fmul fast float
    // STRICT: fmul fast float
    f[i] = c;
  }
  // Instead the inner pragma's effect ends with its braces and the outer
  // math_mode(safe) is back in force.
  f[i + 1] = a * b;
  // FM: fmul float
  // STRICT: fmul float
  f[i + 2] = a + b;
  // FM: fadd float
  // STRICT: fadd float
}

// The function body's closing restore also keeps the pragma from leaking
// into the next function; this one re-evaluates in the line's default mode.
kernel void leak_check(device float *f [[buffer(0)]],
                       uint i [[thread_position_in_grid]]) {
  f[i] = f[i] * 2.0f;
  // FM: fmul fast float
  // STRICT: fmul contract float
}

// At file scope there is no enclosing compound statement, so like
// #pragma float_control the pragma lasts from its point to the end of the
// translation unit.
#pragma METAL fp math_mode(precise)
kernel void file_precise(device float *f [[buffer(0)]],
                         uint i [[thread_position_in_grid]]) {
  f[i] = f[i] * f[i + 1];
  // FM: fmul float
  // STRICT: fmul float
}

#pragma METAL fp math_mode(fast)
kernel void file_fast(device float *f [[buffer(0)]],
                      uint i [[thread_position_in_grid]]) {
  f[i] = f[i] * f[i + 1];
  // FM: fmul fast float
  // STRICT: fmul fast float
}
