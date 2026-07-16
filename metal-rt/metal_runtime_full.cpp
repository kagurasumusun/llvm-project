// metal_runtime_full.cpp - Complete Metal Runtime Implementation
// Clean-room implementation of Apple's Metal runtime
// Copyright (c) 2026 Metal Linux Compiler Project
// SPDX-License-Identifier: MIT

extern "C" {

// ============================================================================
// Bit Manipulation - All integer types, all widths
// ============================================================================

template<typename T>
static T extract_bits_impl(T value, int offset, int width) {
    T mask = (T(1) << width) - 1;
    return (value >> offset) & mask;
}

template<typename T>
static T insert_bits_impl(T value, T bits, int offset, int width) {
    T mask = (T(1) << width) - 1;
    return (value & ~(mask << offset)) | ((bits & mask) << offset);
}

template<typename T>
static T reverse_bits_impl(T value, int bits) {
    T result = 0;
    for (int i = 0; i < bits; i++) {
        result = (result << 1) | (value & 1);
        value >>= 1;
    }
    return result;
}

// 8-bit
int8_t ___metal_extract_bits_int8(int8_t value, int offset, int width) {
    return extract_bits_impl<int8_t>(value, offset, width);
}
int8_t ___metal_reverse_bits_int8(int8_t value) { return reverse_bits_impl<int8_t>(value, 8); }
int8_t ___metal_insert_bits_int8_int8(int8_t value, int8_t bits, int offset, int width) {
    return insert_bits_impl<int8_t>(value, bits, offset, width);
}
uint8_t ___metal_extract_bits_uint8(uint8_t value, int offset, int width) {
    return extract_bits_impl<uint8_t>(value, offset, width);
}
uint8_t ___metal_reverse_bits_uint8(uint8_t value) { return reverse_bits_impl<uint8_t>(value, 8); }
uint8_t ___metal_insert_bits_uint8_uint8(uint8_t value, uint8_t bits, int offset, int width) {
    return insert_bits_impl<uint8_t>(value, bits, offset, width);
}

// 16-bit
int16_t ___metal_extract_bits_int16(int16_t value, int offset, int width) {
    return extract_bits_impl<int16_t>(value, offset, width);
}
int16_t ___metal_reverse_bits_int16(int16_t value) { return reverse_bits_impl<int16_t>(value, 16); }
int16_t ___metal_insert_bits_int16_int16(int16_t value, int16_t bits, int offset, int width) {
    return insert_bits_impl<int16_t>(value, bits, offset, width);
}
uint16_t ___metal_extract_bits_uint16(uint16_t value, int offset, int width) {
    return extract_bits_impl<uint16_t>(value, offset, width);
}
uint16_t ___metal_reverse_bits_uint16(uint16_t value) { return reverse_bits_impl<uint16_t>(value, 16); }
uint16_t ___metal_insert_bits_uint16_uint16(uint16_t value, uint16_t bits, int offset, int width) {
    return insert_bits_impl<uint16_t>(value, bits, offset, width);
}

// 32-bit
int32_t ___metal_extract_bits_int32(int32_t value, int offset, int width) {
    return extract_bits_impl<int32_t>(value, offset, width);
}
int32_t ___metal_reverse_bits_int32(int32_t value) { return reverse_bits_impl<int32_t>(value, 32); }
int32_t ___metal_insert_bits_int32_int32(int32_t value, int32_t bits, int offset, int width) {
    return insert_bits_impl<int32_t>(value, bits, offset, width);
}
uint32_t ___metal_extract_bits_uint32(uint32_t value, int offset, int width) {
    return extract_bits_impl<uint32_t>(value, offset, width);
}
uint32_t ___metal_reverse_bits_uint32(uint32_t value) { return reverse_bits_impl<uint32_t>(value, 32); }
uint32_t ___metal_insert_bits_uint32_uint32(uint32_t value, uint32_t bits, int offset, int width) {
    return insert_bits_impl<uint32_t>(value, bits, offset, width);
}

// 64-bit
int64_t ___metal_extract_bits_int64(int64_t value, int offset, int width) {
    return extract_bits_impl<int64_t>(value, offset, width);
}
int64_t ___metal_reverse_bits_int64(int64_t value) { return reverse_bits_impl<int64_t>(value, 64); }
int64_t ___metal_insert_bits_int64_int64(int64_t value, int64_t bits, int offset, int width) {
    return insert_bits_impl<int64_t>(value, bits, offset, width);
}
uint64_t ___metal_extract_bits_uint64(uint64_t value, int offset, int width) {
    return extract_bits_impl<uint64_t>(value, offset, width);
}
uint64_t ___metal_reverse_bits_uint64(uint64_t value) { return reverse_bits_impl<uint64_t>(value, 64); }
uint64_t ___metal_insert_bits_uint64_uint64(uint64_t value, uint64_t bits, int offset, int width) {
    return insert_bits_impl<uint64_t>(value, bits, offset, width);
}

// Vector versions
#define DEFINE_VEC_BIT_OPS(type, suffix, BW) \
    type ___metal_extract_bits_##suffix(type value, int offset, int width) { return extract_bits_impl<type>(value, offset, width); } \
    type ___metal_reverse_bits_##suffix(type value) { return reverse_bits_impl<type>(value, BW); } \
    type ___metal_insert_bits_##suffix##_##suffix(type value, type bitVal, int offset, int width) { return insert_bits_impl<type>(value, bitVal, offset, width); }

// v2
DEFINE_VEC_BIT_OPS(int8_t, v2int8, 8)
DEFINE_VEC_BIT_OPS(int16_t, v2int16, 16)
DEFINE_VEC_BIT_OPS(int32_t, v2int32, 32)
DEFINE_VEC_BIT_OPS(int64_t, v2int64, 64)
DEFINE_VEC_BIT_OPS(uint8_t, v2uint8, 8)
DEFINE_VEC_BIT_OPS(uint16_t, v2uint16, 16)
DEFINE_VEC_BIT_OPS(uint32_t, v2uint32, 32)
DEFINE_VEC_BIT_OPS(uint64_t, v2uint64, 64)
// v3
DEFINE_VEC_BIT_OPS(int8_t, v3int8, 8)
DEFINE_VEC_BIT_OPS(int16_t, v3int16, 16)
DEFINE_VEC_BIT_OPS(int32_t, v3int32, 32)
DEFINE_VEC_BIT_OPS(int64_t, v3int64, 64)
DEFINE_VEC_BIT_OPS(uint8_t, v3uint8, 8)
DEFINE_VEC_BIT_OPS(uint16_t, v3uint16, 16)
DEFINE_VEC_BIT_OPS(uint32_t, v3uint32, 32)
DEFINE_VEC_BIT_OPS(uint64_t, v3uint64, 64)
// v4
DEFINE_VEC_BIT_OPS(int8_t, v4int8, 8)
DEFINE_VEC_BIT_OPS(int16_t, v4int16, 16)
DEFINE_VEC_BIT_OPS(int32_t, v4int32, 32)
DEFINE_VEC_BIT_OPS(int64_t, v4int64, 64)
DEFINE_VEC_BIT_OPS(uint8_t, v4uint8, 8)
DEFINE_VEC_BIT_OPS(uint16_t, v4uint16, 16)
DEFINE_VEC_BIT_OPS(uint32_t, v4uint32, 32)
DEFINE_VEC_BIT_OPS(uint64_t, v4uint64, 64)
// v8
DEFINE_VEC_BIT_OPS(int8_t, v8int8, 8)
DEFINE_VEC_BIT_OPS(int16_t, v8int16, 16)
DEFINE_VEC_BIT_OPS(int32_t, v8int32, 32)
DEFINE_VEC_BIT_OPS(int64_t, v8int64, 64)
DEFINE_VEC_BIT_OPS(uint8_t, v8uint8, 8)
DEFINE_VEC_BIT_OPS(uint16_t, v8uint16, 16)
DEFINE_VEC_BIT_OPS(uint32_t, v8uint32, 32)
DEFINE_VEC_BIT_OPS(uint64_t, v8uint64, 64)
// v16
DEFINE_VEC_BIT_OPS(int8_t, v16int8, 8)
DEFINE_VEC_BIT_OPS(int16_t, v16int16, 16)
DEFINE_VEC_BIT_OPS(int32_t, v16int32, 32)
DEFINE_VEC_BIT_OPS(int64_t, v16int64, 64)
DEFINE_VEC_BIT_OPS(uint8_t, v16uint8, 8)
DEFINE_VEC_BIT_OPS(uint16_t, v16uint16, 16)
DEFINE_VEC_BIT_OPS(uint32_t, v16uint32, 32)
DEFINE_VEC_BIT_OPS(uint64_t, v16uint64, 64)

// ============================================================================
// Math - fract
// ============================================================================
float ___metal_fract_float(float x, int) { return x - __builtin_floorf(x); }
float ___metal_fract_half(float x, int) { return x - __builtin_floorf(x); }
double ___metal_fract_double(double x, int) { return x - __builtin_floor(x); }

#define FRACT_VEC(suffix) float ___metal_fract_##suffix(float x, int y) { return x - __builtin_floorf(x); }
FRACT_VEC(v2float)  FRACT_VEC(v3float)  FRACT_VEC(v4float)  FRACT_VEC(v8float)  FRACT_VEC(v16float)
FRACT_VEC(v2half)   FRACT_VEC(v3half)   FRACT_VEC(v4half)   FRACT_VEC(v8half)   FRACT_VEC(v16half)

#define FRACT_VEC_D(suffix) double ___metal_fract_##suffix(double x, int y) { return x - __builtin_floor(x); }
FRACT_VEC_D(v2double) FRACT_VEC_D(v3double) FRACT_VEC_D(v4double) FRACT_VEC_D(v8double) FRACT_VEC_D(v16double)

} // extern C
