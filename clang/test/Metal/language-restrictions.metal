// Metal language restrictions: the set of C++ constructs MSL bans and the
// exact wording of each rejection.
//
// Every expected text below is transcribed verbatim from Apple's compiler
// output in reference/metal-ast-macos-air64/log (fixtures cf_goto,
// cf_asm_inline, cf_static_local, type_register, cf_throw_in_kernel,
// cf_catch_in_kernel, cxx_006_new_delete, sig_noexcept_kernel,
// cx_009_lambda_capture_by_ref, era_function_pointer_before_metal21,
// cf_pointer_to_member_function, misc_c_style_cast_addrspace,
// misc_reinterpret_cast_addrspace, repr_packed_suite, cf_main_function,
// type_thread_local_static).
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal4.0 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x metal -triple air64_v20-apple-macosx10.13.0 \
// RUN:   -std=macos-metal1.1 -fsyntax-only -verify -DPRE21 %s

typedef unsigned int uint;
typedef __attribute__((ext_vector_type(3))) float packed_float3_typedef_check;
typedef __attribute__((packed_vector_type(3))) float packed_float3;

int helper_int(int v) { return v; }

struct S { void m() {} int x; };

kernel void goto_kernel(device int *o [[buffer(0)]]) {
  goto end;                       // expected-error {{'goto' is not supported in Metal}}
  end: o[0] = 1;                  // expected-error {{labeled statements are not supported in Metal}}
}

kernel void asm_kernel(device int *o [[buffer(0)]]) {
  asm("nop");                     // expected-error {{illegal asm statement}}
}

kernel void static_kernel(device int *o [[buffer(0)]]) {
  static int s = 0;               // expected-error {{variables in function scope cannot be declared static}}
  o[0] = s;
}

kernel void register_kernel(device uint *o [[buffer(0)]]) {
  register int r = 1;             // expected-error {{Metal does not support the 'register' storage class specifier}}
  o[0] = uint(r);
}

kernel void exception_kernel(device int *o [[buffer(0)]]) {
  try {                           // expected-error {{'try' is not supported in Metal}}
    throw 42;                     // expected-error {{'throw' is not supported in Metal}}
  } catch (int) {
    o[0] = 1;
  }
}

void noexcept_helper() noexcept {} // expected-error {{'noexcept' is not supported in Metal}}

kernel void new_delete_kernel(device int *o [[buffer(0)]]) {
  device int *p = 0;
  p = new int(1);                 // expected-error {{'new' is not supported in Metal}}
  delete p;                       // expected-error {{'delete' is not supported in Metal}}
}

#ifdef PRE21
kernel void fnptr_kernel(device int *o [[buffer(0)]]) {
  int (*fp)(int) = helper_int;    // expected-error {{pointers to functions are not allowed}}
  void (S::*mp)() = &S::m;        // expected-error {{pointers to functions are not allowed}} \
                                  // expected-error {{taking address of function is not allowed}}
  auto l = [](int v) { return v; }; // expected-error {{lambda expressions are not supported in Metal}}
  o[0] = fp(1);
}
#else
// Function pointers, member pointers and lambdas are legal from Metal 2.1
// (measured boundary) and Metal 3.2 (specification) onwards; metal4.0
// accepts all of them.
kernel void fnptr_ok_kernel(device int *o [[buffer(0)]]) {
  int (*fp)(int) = helper_int;
  void (S::*mf)() = &S::m;
  int S::*md = &S::x;
  auto l = [](int v) { return v; };
  (void)mf; (void)md; (void)l;
  o[0] = fp(1);
}
#endif

kernel void cast_kernel(device int *o [[buffer(0)]]) {
  thread int x = 1;
  device int *p = (device int *)&x;
  // expected-error@-1 {{C-style cast from 'int *' to 'device int *' converts between mismatching address spaces}}
  device int *q = reinterpret_cast<device int *>(&x);
  // expected-error@-1 {{reinterpret_cast from 'int *' to 'device int *' is not allowed}}
  o[0] = *p + *q;
}

struct PackedMembers { packed_float3 c; };
kernel void packed_swizzle_kernel(device int *o [[buffer(0)]],
                                  constant PackedMembers &p [[buffer(1)]]) {
  float v = p.c.x;                // expected-error {{illegal vector component name 'x'}}
  o[0] = int(v);
}

int main() { return 0; }          // expected-error {{non-qualified function cannot be called 'main'}}
