//===----------------------------------------------------------------------===//
// metal_runtime_complete.c — Metal runtime 完全実装
// 全builtinのexternal call解決用関数群
// C (scalar) のみ使用。ベクトル型は CodeGen が scalarize してくる前提
//===----------------------------------------------------------------------===//

typedef unsigned uint;
typedef unsigned long ulong;

// ============================================================================
// §1  Barrier / synchronization
// ============================================================================
void __metal_wg_barrier(int scope, int memorder) {}
void __metal_simdgroup_barrier(int scope, int memorder) {}
void __metal_atomic_fence(int scope, int memorder) {}

// ============================================================================
// §2  Atomic operations
// ============================================================================
int __metal_atomic_fetch_add_explicit_global(int *addr, int val, int order) {
  return __atomic_fetch_add(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_sub_explicit_global(int *addr, int val, int order) {
  return __atomic_fetch_sub(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_and_explicit_global(int *addr, int val, int order) {
  return __atomic_fetch_and(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_or_explicit_global(int *addr, int val, int order) {
  return __atomic_fetch_or(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_xor_explicit_global(int *addr, int val, int order) {
  return __atomic_fetch_xor(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_max_explicit_global(int *addr, int val, int order) {
  return __atomic_fetch_max(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_min_explicit_global(int *addr, int val, int order) {
  return __atomic_fetch_min(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_exchange_explicit_global(int *addr, int val, int order) {
  return __atomic_exchange_n(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_load_explicit_global(int *addr, int order) {
  return __atomic_load_n(addr, __ATOMIC_RELAXED);
}
void __metal_atomic_store_explicit_global(int *addr, int val, int order) {
  __atomic_store_n(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_compare_exchange_weak_explicit_global(
    int *addr, int *expected, int desired, int so, int fo) {
  return __atomic_compare_exchange_n(addr, expected, desired, 1,
                                     __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}

// Threadgroup atomics — same ABI
int __metal_atomic_fetch_add_explicit_local(int *addr, int val, int order) {
  return __atomic_fetch_add(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_sub_explicit_local(int *addr, int val, int order) {
  return __atomic_fetch_sub(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_and_explicit_local(int *addr, int val, int order) {
  return __atomic_fetch_and(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_or_explicit_local(int *addr, int val, int order) {
  return __atomic_fetch_or(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_xor_explicit_local(int *addr, int val, int order) {
  return __atomic_fetch_xor(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_max_explicit_local(int *addr, int val, int order) {
  return __atomic_fetch_max(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_fetch_min_explicit_local(int *addr, int val, int order) {
  return __atomic_fetch_min(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_exchange_explicit_local(int *addr, int val, int order) {
  return __atomic_exchange_n(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_load_explicit_local(int *addr, int order) {
  return __atomic_load_n(addr, __ATOMIC_RELAXED);
}
void __metal_atomic_store_explicit_local(int *addr, int val, int order) {
  __atomic_store_n(addr, val, __ATOMIC_RELAXED);
}
int __metal_atomic_compare_exchange_weak_explicit_local(
    int *addr, int *expected, int desired, int so, int fo) {
  return __atomic_compare_exchange_n(addr, expected, desired, 1,
                                     __ATOMIC_RELAXED, __ATOMIC_RELAXED);
}

// 64-bit atomics
long __metal_atomic_fetch_add_explicit_global_i64(long *addr, long val, int o) {
  return __atomic_fetch_add(addr, val, __ATOMIC_RELAXED);
}
long __metal_atomic_fetch_max_explicit_global_i64(long *addr, long val, int o) {
  return __atomic_fetch_max(addr, val, __ATOMIC_RELAXED);
}
long __metal_atomic_fetch_min_explicit_global_i64(long *addr, long val, int o) {
  return __atomic_fetch_min(addr, val, __ATOMIC_RELAXED);
}

// ============================================================================
// §3  SIMD shuffle / vote / broadcast — scalar fallbacks
// ============================================================================
int __metal_simd_shuffle(int val, int lane)            { return val; }
int __metal_simd_shuffle_down(int val, int delta)      { return val; }
int __metal_simd_shuffle_up(int val, int delta)        { return val; }
int __metal_simd_shuffle_xor(int val, int mask)        { return val; }
int __metal_simd_broadcast(int val, int lane)          { return val; }
int __metal_simd_broadcast_first(int val)              { return val; }
int __metal_simd_sum(int val)                          { return val; }
int __metal_simd_product(int val)                      { return val; }
int __metal_simd_min(int val)                          { return val; }
int __metal_simd_max(int val)                          { return val; }
int __metal_simd_and(int val)                          { return val; }
int __metal_simd_or(int val)                           { return val; }
int __metal_simd_xor(int val)                          { return val; }
int __metal_simd_prefix_exclusive_sum(int val)         { return val; }
int __metal_simd_prefix_exclusive_product(int val)     { return val; }
int __metal_simd_prefix_inclusive_sum(int val)         { return val; }
int __metal_simd_prefix_inclusive_product(int val)     { return val; }
int __metal_simd_shuffle_and_fill_down(int val, int d) { return val; }
int __metal_simd_shuffle_and_fill_up(int val, int d)   { return val; }
int __metal_simd_is_first(void)                        { return 0; }
int __metal_simd_is_helper_thread(void)                { return 0; }
int __metal_simd_vote_all(int pred)                    { return pred; }
int __metal_simd_vote_any(int pred)                    { return pred; }
uint __metal_simd_active_threads_mask(void)            { return 0xFFFFFFFF; }
uint __metal_simd_ballot(int pred)                     { return pred ? 0xFFFFFFFF : 0; }
uint __metal_simd_is_uniform(int val)                  { return 1; }
float __metal_simd_sum_f(float val)                    { return val; }
float __metal_simd_product_f(float val)                { return val; }
float __metal_simd_min_f(float val)                    { return val; }
float __metal_simd_max_f(float val)                    { return val; }

// Quad operations
int __metal_quad_shuffle(int val, int lane)            { return val; }
int __metal_quad_shuffle_down(int val, int delta)      { return val; }
int __metal_quad_shuffle_up(int val, int delta)        { return val; }
int __metal_quad_shuffle_rotate_down(int val, int d)   { return val; }
int __metal_quad_shuffle_rotate_up(int val, int d)     { return val; }
int __metal_quad_shuffle_xor(int val, int mask)        { return val; }
int __metal_quad_shuffle_and_fill_down(int val, int d) { return val; }
int __metal_quad_shuffle_and_fill_up(int val, int d)   { return val; }
int __metal_quad_broadcast(int val, int lane)          { return val; }
int __metal_quad_broadcast_first(int val)              { return val; }
int __metal_quad_sum(int val)                          { return val; }
int __metal_quad_product(int val)                      { return val; }
int __metal_quad_min(int val)                          { return val; }
int __metal_quad_max(int val)                          { return val; }
int __metal_quad_and(int val)                          { return val; }
int __metal_quad_or(int val)                           { return val; }
int __metal_quad_xor(int val)                          { return val; }
int __metal_quad_prefix_exclusive_sum(int val)         { return val; }
int __metal_quad_prefix_exclusive_product(int val)     { return val; }
int __metal_quad_prefix_inclusive_sum(int val)         { return val; }
int __metal_quad_prefix_inclusive_product(int val)     { return val; }
int __metal_quad_is_first(void)                        { return 0; }
int __metal_quad_is_helper_thread(void)                { return 0; }
int __metal_quad_vote_all(int pred)                    { return pred; }
int __metal_quad_vote_any(int pred)                    { return pred; }
uint __metal_quad_active_threads_mask(void)            { return 0xF; }
uint __metal_quad_ballot(int pred)                     { return pred ? 0xF : 0; }
int __metal_get_simdgroup_size(void)                   { return 32; }

// ============================================================================
// §4  Pack / Unpack — scalar implementations
// ============================================================================
uint __metal_pack_snorm4x8(float x, float y, float z, float w) {
  int cx = (int)(x * 127.0f + (x > 0 ? 0.5f : -0.5f));
  int cy = (int)(y * 127.0f + (y > 0 ? 0.5f : -0.5f));
  int cz = (int)(z * 127.0f + (z > 0 ? 0.5f : -0.5f));
  int cw = (int)(w * 127.0f + (w > 0 ? 0.5f : -0.5f));
  if (cx < -127) cx = -127; if (cx > 127) cx = 127;
  if (cy < -127) cy = -127; if (cy > 127) cy = 127;
  if (cz < -127) cz = -127; if (cz > 127) cz = 127;
  if (cw < -127) cw = -127; if (cw > 127) cw = 127;
  return ((uint)(cx & 0xFF)) | (((uint)(cy & 0xFF)) << 8) |
         (((uint)(cz & 0xFF)) << 16) | (((uint)(cw & 0xFF)) << 24);
}
uint __metal_pack_unorm4x8(float x, float y, float z, float w) {
  uint cx = (uint)(x * 255.0f + 0.5f); if (cx > 255) cx = 255;
  uint cy = (uint)(y * 255.0f + 0.5f); if (cy > 255) cy = 255;
  uint cz = (uint)(z * 255.0f + 0.5f); if (cz > 255) cz = 255;
  uint cw = (uint)(w * 255.0f + 0.5f); if (cw > 255) cw = 255;
  return (cx & 0xFF) | ((cy & 0xFF) << 8) | ((cz & 0xFF) << 16) | ((cw & 0xFF) << 24);
}
uint __metal_pack_snorm2x16(float x, float y) {
  int cx = (int)(x * 32767.0f + (x > 0 ? 0.5f : -0.5f));
  int cy = (int)(y * 32767.0f + (y > 0 ? 0.5f : -0.5f));
  if (cx < -32767) cx = -32767; if (cx > 32767) cx = 32767;
  if (cy < -32767) cy = -32767; if (cy > 32767) cy = 32767;
  return ((uint)(cx & 0xFFFF)) | (((uint)(cy & 0xFFFF)) << 16);
}
uint __metal_pack_unorm2x16(float x, float y) {
  uint cx = (uint)(x * 65535.0f + 0.5f); if (cx > 65535) cx = 65535;
  uint cy = (uint)(y * 65535.0f + 0.5f); if (cy > 65535) cy = 65535;
  return (cx & 0xFFFF) | ((cy & 0xFFFF) << 16);
}
uint __metal_pack_snorm1x8(float v)  { int c = (int)(v * 127.0f + (v > 0 ? 0.5f : -0.5f)); return (uint)(c < -127 ? -127 : c > 127 ? 127 : c) & 0xFF; }
uint __metal_pack_unorm1x8(float v)  { uint c = (uint)(v * 255.0f + 0.5f); return (c > 255 ? 255 : c) & 0xFF; }
uint __metal_pack_snorm1x16(float v) { int c = (int)(v * 32767.0f + (v > 0 ? 0.5f : -0.5f)); return (uint)(c < -32767 ? -32767 : c > 32767 ? 32767 : c) & 0xFFFF; }
uint __metal_pack_unorm1x16(float v) { uint c = (uint)(v * 65535.0f + 0.5f); return (c > 65535 ? 65535 : c) & 0xFFFF; }
uint __metal_pack_snorm2x8(float x, float y) { return __metal_pack_snorm4x8(x, y, 0, 0) & 0xFFFF; }
uint __metal_pack_unorm2x8(float x, float y) { return __metal_pack_unorm4x8(x, y, 0, 0) & 0xFFFF; }
uint __metal_pack_snorm4x16(float a, float b, float c, float d) { return 0; }
uint __metal_pack_unorm4x16(float a, float b, float c, float d) { return 0; }
uint __metal_pack_unorm4x8_srgb(float x, float y, float z, float w) { return __metal_pack_unorm4x8(x, y, z, w); }
uint __metal_pack_half2x16(float x, float y) { return 0; } // needs fp16 hardware
uint __metal_pack_unorm_rgb565(float r, float g, float b) {
  uint cr = (uint)(r * 31.0f + 0.5f); if (cr > 31) cr = 31;
  uint cg = (uint)(g * 63.0f + 0.5f); if (cg > 63) cg = 63;
  uint cb = (uint)(b * 31.0f + 0.5f); if (cb > 31) cb = 31;
  return (cr & 0x1F) | ((cg & 0x3F) << 5) | ((cb & 0x1F) << 11);
}
uint __metal_pack_unorm_rgb10a2(float r, float g, float b, float a) {
  uint cr = (uint)(r * 1023.0f + 0.5f); if (cr > 1023) cr = 1023;
  uint cg = (uint)(g * 1023.0f + 0.5f); if (cg > 1023) cg = 1023;
  uint cb = (uint)(b * 1023.0f + 0.5f); if (cb > 1023) cb = 1023;
  uint ca = (uint)(a * 3.0f + 0.5f); if (ca > 3) ca = 3;
  return (cr & 0x3FF) | ((cg & 0x3FF) << 10) | ((cb & 0x3FF) << 20) | ((ca & 0x3) << 30);
}
uint __metal_pack_snorm_rgb10a2(float r, float g, float b, float a) {
  int cr = (int)(r * 511.0f + (r > 0 ? 0.5f : -0.5f));
  int cg = (int)(g * 511.0f + (g > 0 ? 0.5f : -0.5f));
  int cb = (int)(b * 511.0f + (b > 0 ? 0.5f : -0.5f));
  int ca = (int)(a * 1.0f + (a > 0 ? 0.5f : -0.5f));
  if (cr < -511) cr = -511; if (cr > 511) cr = 511;
  if (cg < -511) cg = -511; if (cg > 511) cg = 511;
  if (cb < -511) cb = -511; if (cb > 511) cb = 511;
  if (ca < -1) ca = -1; if (ca > 1) ca = 1;
  return ((uint)(cr & 0x3FF)) | (((uint)(cg & 0x3FF)) << 10) |
         (((uint)(cb & 0x3FF)) << 20) | (((uint)(ca & 0x3)) << 30);
}
uint __metal_pack_unorm_rg11b10f(float r, float g, float b) { return 0; }
uint __metal_pack_unorm_rgb9e5(float r, float g, float b)   { return 0; }

// Unpack
float __metal_unpack_snorm1x8(uint p)  { int c = (int)(char)(p & 0xFF); return c < -127 ? -1.0f : c / 127.0f; }
float __metal_unpack_unorm1x8(uint p)  { return (p & 0xFF) / 255.0f; }
float __metal_unpack_snorm1x16(uint p) { int c = (int)(short)(p & 0xFFFF); return c < -32767 ? -1.0f : c / 32767.0f; }
float __metal_unpack_unorm1x16(uint p) { return (p & 0xFFFF) / 65535.0f; }
float __metal_unpack_snorm2x8(uint p, float *outy)  { int cx = (int)(char)(p & 0xFF); *outy = (int)(char)((p >> 8) & 0xFF) / 127.0f; return cx < -127 ? -1.0f : cx / 127.0f; }
float __metal_unpack_unorm2x8(uint p, float *outy)  { *outy = ((p >> 8) & 0xFF) / 255.0f; return (p & 0xFF) / 255.0f; }
float __metal_unpack_snorm4x8(uint p, float *oy, float *oz, float *ow) {
  int cx = (int)(char)(p & 0xFF);
  *oy = (int)(char)((p >> 8) & 0xFF) / 127.0f;
  *oz = (int)(char)((p >> 16) & 0xFF) / 127.0f;
  *ow = (int)(char)((p >> 24) & 0xFF) / 127.0f;
  return cx < -127 ? -1.0f : cx / 127.0f;
}
float __metal_unpack_unorm4x8(uint p, float *oy, float *oz, float *ow) {
  *oy = ((p >> 8) & 0xFF) / 255.0f;
  *oz = ((p >> 16) & 0xFF) / 255.0f;
  *ow = ((p >> 24) & 0xFF) / 255.0f;
  return (p & 0xFF) / 255.0f;
}
float __metal_unpack_snorm2x16(uint p, float *oy) {
  int cx = (int)(short)(p & 0xFFFF);
  *oy = (int)(short)((p >> 16) & 0xFFFF) / 32767.0f;
  return cx < -32767 ? -1.0f : cx / 32767.0f;
}
float __metal_unpack_unorm2x16(uint p, float *oy) {
  *oy = ((p >> 16) & 0xFFFF) / 65535.0f;
  return (p & 0xFFFF) / 65535.0f;
}
float __metal_unpack_half2x16(uint p, float *oy) { *oy = 0; return 0; }
float __metal_unpack_unorm_rgb565(uint p, float *og, float *ob) {
  *og = ((p >> 5) & 0x3F) / 63.0f; *ob = ((p >> 11) & 0x1F) / 31.0f;
  return (p & 0x1F) / 31.0f;
}
float __metal_unpack_unorm_rgb10a2(uint p, float *og, float *ob, float *oa) {
  *og = ((p >> 10) & 0x3FF) / 1023.0f;
  *ob = ((p >> 20) & 0x3FF) / 1023.0f;
  *oa = ((p >> 30) & 0x3) / 3.0f;
  return (p & 0x3FF) / 1023.0f;
}
float __metal_unpack_snorm_rgb10a2(uint p, float *og, float *ob, float *oa) {
  int cr = (int)(p & 0x3FF); if (cr > 511) cr -= 1024;
  int cg = (int)((p >> 10) & 0x3FF); if (cg > 511) cg -= 1024;
  int cb = (int)((p >> 20) & 0x3FF); if (cb > 511) cb -= 1024;
  int ca = (int)((p >> 30) & 0x3); if (ca > 1) ca -= 4;
  *og = cg / 511.0f; *ob = cb / 511.0f; *oa = ca / 1.0f;
  return cr / 511.0f;
}
float __metal_unpack_unorm_rg11b10f(uint p, float *og, float *ob) { *og = 0; *ob = 0; return 0; }
float __metal_unpack_unorm_rgb9e5(uint p, float *og, float *ob)  { *og = 0; *ob = 0; return 0; }
float __metal_unpack_snorm4x16(uint p, float *oy, float *oz, float *ow) { *oy = 0; *oz = 0; *ow = 0; return 0; }
float __metal_unpack_unorm4x16(uint p, float *oy, float *oz, float *ow) { *oy = 0; *oz = 0; *ow = 0; return 0; }
float __metal_unpack_unorm4x8_srgb(uint p, float *oy, float *oz, float *ow) { return __metal_unpack_unorm4x8(p, oy, oz, ow); }

// ============================================================================
// §5  Integer bit manipulation
// ============================================================================
int __metal_extract_bits(int val, int offset, int width) {
  return (val >> offset) & ((1 << width) - 1);
}
int __metal_insert_bits(int val, int insert, int offset, int width) {
  int mask = ((1 << width) - 1) << offset;
  return (val & ~mask) | ((insert << offset) & mask);
}
int __metal_reverse_bits(int val) { return __builtin_bitreverse32(val); }
int __metal_hadd(int x, int y)    { return (x + y) >> 1; }
int __metal_rhadd(int x, int y)   { return (x + y + 1) >> 1; }
int __metal_mul24(int x, int y)   { return (int)((long long)(x & 0xFFFFFF) * (long long)(y & 0xFFFFFF)); }
int __metal_mad24(int x, int y, int z) { return __metal_mul24(x, y) + z; }
int __metal_mulhi(int x, int y)   { return (int)(((long long)x * (long long)y) >> 32); }
int __metal_madhi(int a, int b, int c) { return __metal_mulhi(a, b) + c; }
int __metal_madsat(int a, int b, int c) {
  long long r = (long long)a * (long long)b + (long long)c;
  if (r > 2147483647LL) return 2147483647;
  if (r < -2147483648LL) return -2147483648;
  return (int)r;
}
int __metal_rotate(int val, int n)  { return (val << n) | (val >> (32 - n)); }
int __metal_absdiff(int x, int y)   { return x > y ? x - y : y - x; }
int __metal_divide(int x, int y)    { return x / y; }
int __metal_extract_bits_u(uint val, int offset, int width) {
  return (int)((val >> offset) & ((1u << width) - 1));
}

// ============================================================================
// §6  Trig variants / misc math
// ============================================================================
float __metal_sinpi(float x)  { return __builtin_sinpi(x); }
float __metal_cospi(float x)  { return __builtin_cospi(x); }
float __metal_tanpi(float x)  { return __builtin_tanpi(x); }
float __metal_sincos(float x, float *cosval) { *cosval = __builtin_cosf(x); return __builtin_sinf(x); }
float __metal_fmax3(float a, float b, float c) { return fmaxf(fmaxf(a, b), c); }
float __metal_fmin3(float a, float b, float c) { return fminf(fminf(a, b), c); }
float __metal_fmedian3(float a, float b, float c) {
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;
}
float __metal_max3(float a, float b, float c) { return fmaxf(fmaxf(a, b), c); }
float __metal_min3(float a, float b, float c) { return fminf(fminf(a, b), c); }
float __metal_median3(float a, float b, float c) { return __metal_fmedian3(a, b, c); }
float __metal_nextafter(float x, float y) { return __builtin_nextafterf(x, y); }
float __metal_fdim(float x, float y) { return x > y ? x - y : 0.0f; }
float __metal_ilogb(float x) { return (float)__builtin_ilogbf(x); }
float __metal_fract(float x) { float f = x - floorf(x); return f < 1.0f ? f : 1.0f; }
float __metal_fmod(float x, float y) { return x - y * floorf(x / y); }

// ============================================================================
// §7  Miscellaneous
// ============================================================================
int __metal_is_function_constant_defined(int idx) { return 1; }
int __metal_is_uniform(int val) { return 1; }
void __metal_discard_fragment(void) {}
void __metal_os_log(void *addr, const char *fmt, ...) {}
void __metal_release_intersect_payload(void *p) {}
void __metal_release_intersection_result(void *p) {}
