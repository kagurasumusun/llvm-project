//===----------------------------------------------------------------------===//
// air_intrinsics.h — AIR intrinsic declarations for runtime library
// These are the LLVM intrinsic functions that Apple's Metal runtime provides.
// Our cleanroom implementation calls the same intrinsics.
//===----------------------------------------------------------------------===//

#ifndef AIR_INTRINSICS_H
#define AIR_INTRINSICS_H

// ---- Math intrinsics ----
float air_sin_f32(float) __asm("air.sin.f32");
float air_cos_f32(float) __asm("air.cos.f32");
float air_tan_f32(float) __asm("air.tan.f32");
float air_asin_f32(float) __asm("air.asin.f32");
float air_acos_f32(float) __asm("air.acos.f32");
float air_atan_f32(float) __asm("air.atan.f32");
float air_atan2_f32(float, float) __asm("air.atan2.f32");
float air_sinh_f32(float) __asm("air.sinh.f32");
float air_cosh_f32(float) __asm("air.cosh.f32");
float air_tanh_f32(float) __asm("air.tanh.f32");
float air_exp_f32(float) __asm("air.exp.f32");
float air_exp2_f32(float) __asm("air.exp2.f32");
float air_exp10_f32(float) __asm("air.exp10.f32");
float air_log_f32(float) __asm("air.log.f32");
float air_log2_f32(float) __asm("air.log2.f32");
float air_log10_f32(float) __asm("air.log10.f32");
float air_sqrt_f32(float) __asm("air.sqrt.f32");
float air_pow_f32(float, float) __asm("air.pow.f32");
float air_fabs_f32(float) __asm("air.fabs.f32");
float air_floor_f32(float) __asm("air.floor.f32");
float air_ceil_f32(float) __asm("air.ceil.f32");
float air_trunc_f32(float) __asm("air.trunc.f32");
float air_rint_f32(float) __asm("air.rint.f32");
float air_round_f32(float) __asm("air.round.f32");
float air_fma_f32(float, float, float) __asm("air.fma.f32");
float air_fmax_f32(float, float) __asm("air.fmax.f32");
float air_fmin_f32(float, float) __asm("air.fmin.f32");
float air_copysign_f32(float, float) __asm("air.copysign.f32");

// ---- Integer intrinsics ----
int air_clz_i32(int) __asm("air.clz.i32");
int air_ctz_i32(int) __asm("air.ctz.i32");
int air_popcount_i32(int) __asm("air.popcount.i32");
int air_abs_diff_i32(int, int) __asm("air.abs_diff.s.i32");
int air_add_sat_i32(int, int) __asm("air.add_sat.s.i32");
int air_sub_sat_i32(int, int) __asm("air.sub_sat.s.i32");
int air_mul_hi_i32(int, int) __asm("air.mul_hi.s.i32");
int air_mad_hi_i32(int, int, int) __asm("air.mad_hi.s.i32");

// ---- Barrier ----
void air_wg_barrier(int, int) __asm("air.wg.barrier");

// ---- Atomic intrinsics ----
int air_atomic_global_add_i32(int*, int) __asm("air.atomic.global.add.u.i32");
int air_atomic_global_sub_i32(int*, int) __asm("air.atomic.global.sub.u.i32");
int air_atomic_global_xchg_i32(int*, int) __asm("air.atomic.global.xchg.u.i32");
int air_atomic_global_cmpxchg_weak_i32(int*, int*, int) __asm("air.atomic.global.cmpxchg.weak");
int air_atomic_global_max_i32(int*, int) __asm("air.atomic.global.max.s.i32");
int air_atomic_global_min_i32(int*, int) __asm("air.atomic.global.min.s.i32");
int air_atomic_global_and_i32(int*, int) __asm("air.atomic.global.and.u.i32");
int air_atomic_global_or_i32(int*, int) __asm("air.atomic.global.or.u.i32");
int air_atomic_global_xor_i32(int*, int) __asm("air.atomic.global.xor.u.i32");

// ---- Thread position ----
typedef struct { unsigned x, y, z; } uint3;
uint3 air_thread_position_in_grid() __asm("air.thread_position_in_grid");
uint3 air_thread_position_in_threadgroup() __asm("air.thread_position_in_threadgroup");
uint3 air_threadgroup_position_in_grid() __asm("air.threadgroup_position_in_grid");
uint3 air_threads_per_threadgroup() __asm("air.threads_per_threadgroup");
uint air_thread_index_in_threadgroup() __asm("air.thread_index_in_threadgroup");

#endif // AIR_INTRINSICS_H
