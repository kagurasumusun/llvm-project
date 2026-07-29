// __packed_vector_type__ is an Apple extension with no upstream equivalent.
// The Metal standard library declares the whole packed_* family with it (82
// uses across the reference headers), so it is required to parse them.
//
// The layout rule is tabulated in the Metal Shading Language specification:
// a packed vector occupies exactly its elements and is aligned like one
// element, so packed_float3 is 12/4 where float3 is 16/16.
//
// RUN: %clang_cc1 -x metal -triple air64_v28-apple-macosx26.0.0 \
// RUN:   -std=metal3.2 -fsyntax-only -verify %s
// expected-no-diagnostics

typedef __attribute__((__ext_vector_type__(3)))    float float3;
typedef __attribute__((__packed_vector_type__(3))) float packed_float3;
typedef __attribute__((__packed_vector_type__(2))) float packed_float2;
typedef __attribute__((__packed_vector_type__(4))) float packed_float4;
typedef __attribute__((__packed_vector_type__(3))) unsigned char packed_uchar3;
typedef __attribute__((__packed_vector_type__(3))) short packed_short3;

// Values from the specification's size/alignment table.
static_assert(sizeof(float3)         == 16, "float3 is padded to four elements");
static_assert(alignof(float3)        == 16, "float3 is vector aligned");

static_assert(sizeof(packed_float2)  ==  8, "");
static_assert(alignof(packed_float2) ==  4, "");
static_assert(sizeof(packed_float3)  == 12, "packed_float3 is exactly three floats");
static_assert(alignof(packed_float3) ==  4, "packed_float3 is element aligned");
static_assert(sizeof(packed_float4)  == 16, "");
static_assert(alignof(packed_float4) ==  4, "");
static_assert(sizeof(packed_uchar3)  ==  3, "");
static_assert(alignof(packed_uchar3) ==  1, "");
static_assert(sizeof(packed_short3)  ==  6, "");
static_assert(alignof(packed_short3) ==  2, "");
