// metal_tensor — MSL tensor operations (cleanroom)
#ifndef _METAL_TENSOR_H_
#define _METAL_TENSOR_H_
#include <metal/metal_common>
namespace metal {

struct tensor_t {};
struct tensor_thread_t {};

METAL_FUNC tensor_t get_null_tensor() { return {}; }
METAL_FUNC bool is_null_tensor(tensor_t t) { return true; }
METAL_FUNC uint get_stride_tensor(tensor_t t, int dim) { return 0; }
METAL_FUNC uint get_extent_tensor(tensor_t t, int dim) { return 0; }
METAL_FUNC uint get_descriptor_size_tensor(tensor_t t) { return 0; }
METAL_FUNC device void *get_data_pointer_tensor(tensor_t t) { return nullptr; }
METAL_FUNC tensor_thread_t get_tensor_handle(tensor_t t) { return {}; }
METAL_FUNC tensor_thread_t init_strided_tensor(tensor_t t, uint stride) { return {}; }

template <typename T, int... Dims>
struct tensor {
  METAL_FUNC T get(int idx) const { return T(); }
  METAL_FUNC void set(int idx, T val) {}
  METAL_FUNC int get_rank() const { return sizeof...(Dims); }
  METAL_FUNC int get_extent(int dim) const { return 0; }
  METAL_FUNC int get_stride(int dim) const { return 0; }
  METAL_FUNC void* data_handle() { return nullptr; }
  METAL_FUNC tensor_t handle() const { return {}; }
  METAL_FUNC bool is_null() const { return true; }
};

METAL_FUNC char load_tensor(tensor_thread_t t, int idx) { return char(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, char val) {}
METAL_FUNC char2 load_tensor(tensor_thread_t t, int idx) { return char2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, char2 val) {}
METAL_FUNC char3 load_tensor(tensor_thread_t t, int idx) { return char3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, char3 val) {}
METAL_FUNC char4 load_tensor(tensor_thread_t t, int idx) { return char4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, char4 val) {}
METAL_FUNC char8 load_tensor(tensor_thread_t t, int idx) { return char8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, char8 val) {}
METAL_FUNC char16 load_tensor(tensor_thread_t t, int idx) { return char16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, char16 val) {}
METAL_FUNC short load_tensor(tensor_thread_t t, int idx) { return short(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, short val) {}
METAL_FUNC short2 load_tensor(tensor_thread_t t, int idx) { return short2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, short2 val) {}
METAL_FUNC short3 load_tensor(tensor_thread_t t, int idx) { return short3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, short3 val) {}
METAL_FUNC short4 load_tensor(tensor_thread_t t, int idx) { return short4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, short4 val) {}
METAL_FUNC short8 load_tensor(tensor_thread_t t, int idx) { return short8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, short8 val) {}
METAL_FUNC short16 load_tensor(tensor_thread_t t, int idx) { return short16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, short16 val) {}
METAL_FUNC int load_tensor(tensor_thread_t t, int idx) { return int(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, int val) {}
METAL_FUNC int2 load_tensor(tensor_thread_t t, int idx) { return int2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, int2 val) {}
METAL_FUNC int3 load_tensor(tensor_thread_t t, int idx) { return int3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, int3 val) {}
METAL_FUNC int4 load_tensor(tensor_thread_t t, int idx) { return int4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, int4 val) {}
METAL_FUNC int8 load_tensor(tensor_thread_t t, int idx) { return int8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, int8 val) {}
METAL_FUNC int16 load_tensor(tensor_thread_t t, int idx) { return int16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, int16 val) {}
METAL_FUNC long load_tensor(tensor_thread_t t, int idx) { return long(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, long val) {}
METAL_FUNC long2 load_tensor(tensor_thread_t t, int idx) { return long2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, long2 val) {}
METAL_FUNC long3 load_tensor(tensor_thread_t t, int idx) { return long3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, long3 val) {}
METAL_FUNC long4 load_tensor(tensor_thread_t t, int idx) { return long4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, long4 val) {}
METAL_FUNC long8 load_tensor(tensor_thread_t t, int idx) { return long8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, long8 val) {}
METAL_FUNC long16 load_tensor(tensor_thread_t t, int idx) { return long16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, long16 val) {}
METAL_FUNC uchar load_tensor(tensor_thread_t t, int idx) { return uchar(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uchar val) {}
METAL_FUNC uchar2 load_tensor(tensor_thread_t t, int idx) { return uchar2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uchar2 val) {}
METAL_FUNC uchar3 load_tensor(tensor_thread_t t, int idx) { return uchar3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uchar3 val) {}
METAL_FUNC uchar4 load_tensor(tensor_thread_t t, int idx) { return uchar4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uchar4 val) {}
METAL_FUNC uchar8 load_tensor(tensor_thread_t t, int idx) { return uchar8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uchar8 val) {}
METAL_FUNC uchar16 load_tensor(tensor_thread_t t, int idx) { return uchar16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uchar16 val) {}
METAL_FUNC ushort load_tensor(tensor_thread_t t, int idx) { return ushort(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ushort val) {}
METAL_FUNC ushort2 load_tensor(tensor_thread_t t, int idx) { return ushort2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ushort2 val) {}
METAL_FUNC ushort3 load_tensor(tensor_thread_t t, int idx) { return ushort3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ushort3 val) {}
METAL_FUNC ushort4 load_tensor(tensor_thread_t t, int idx) { return ushort4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ushort4 val) {}
METAL_FUNC ushort8 load_tensor(tensor_thread_t t, int idx) { return ushort8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ushort8 val) {}
METAL_FUNC ushort16 load_tensor(tensor_thread_t t, int idx) { return ushort16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ushort16 val) {}
METAL_FUNC uint load_tensor(tensor_thread_t t, int idx) { return uint(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uint val) {}
METAL_FUNC uint2 load_tensor(tensor_thread_t t, int idx) { return uint2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uint2 val) {}
METAL_FUNC uint3 load_tensor(tensor_thread_t t, int idx) { return uint3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uint3 val) {}
METAL_FUNC uint4 load_tensor(tensor_thread_t t, int idx) { return uint4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uint4 val) {}
METAL_FUNC uint8 load_tensor(tensor_thread_t t, int idx) { return uint8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uint8 val) {}
METAL_FUNC uint16 load_tensor(tensor_thread_t t, int idx) { return uint16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, uint16 val) {}
METAL_FUNC ulong load_tensor(tensor_thread_t t, int idx) { return ulong(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ulong val) {}
METAL_FUNC ulong2 load_tensor(tensor_thread_t t, int idx) { return ulong2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ulong2 val) {}
METAL_FUNC ulong3 load_tensor(tensor_thread_t t, int idx) { return ulong3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ulong3 val) {}
METAL_FUNC ulong4 load_tensor(tensor_thread_t t, int idx) { return ulong4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ulong4 val) {}
METAL_FUNC ulong8 load_tensor(tensor_thread_t t, int idx) { return ulong8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ulong8 val) {}
METAL_FUNC ulong16 load_tensor(tensor_thread_t t, int idx) { return ulong16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, ulong16 val) {}
METAL_FUNC float load_tensor(tensor_thread_t t, int idx) { return float(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, float val) {}
METAL_FUNC float2 load_tensor(tensor_thread_t t, int idx) { return float2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, float2 val) {}
METAL_FUNC float3 load_tensor(tensor_thread_t t, int idx) { return float3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, float3 val) {}
METAL_FUNC float4 load_tensor(tensor_thread_t t, int idx) { return float4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, float4 val) {}
METAL_FUNC float8 load_tensor(tensor_thread_t t, int idx) { return float8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, float8 val) {}
METAL_FUNC float16 load_tensor(tensor_thread_t t, int idx) { return float16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, float16 val) {}
METAL_FUNC half load_tensor(tensor_thread_t t, int idx) { return half(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, half val) {}
METAL_FUNC half2 load_tensor(tensor_thread_t t, int idx) { return half2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, half2 val) {}
METAL_FUNC half3 load_tensor(tensor_thread_t t, int idx) { return half3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, half3 val) {}
METAL_FUNC half4 load_tensor(tensor_thread_t t, int idx) { return half4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, half4 val) {}
METAL_FUNC half8 load_tensor(tensor_thread_t t, int idx) { return half8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, half8 val) {}
METAL_FUNC half16 load_tensor(tensor_thread_t t, int idx) { return half16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, half16 val) {}
METAL_FUNC bfloat load_tensor(tensor_thread_t t, int idx) { return bfloat(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, bfloat val) {}
METAL_FUNC bfloat2 load_tensor(tensor_thread_t t, int idx) { return bfloat2(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, bfloat2 val) {}
METAL_FUNC bfloat3 load_tensor(tensor_thread_t t, int idx) { return bfloat3(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, bfloat3 val) {}
METAL_FUNC bfloat4 load_tensor(tensor_thread_t t, int idx) { return bfloat4(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, bfloat4 val) {}
METAL_FUNC bfloat8 load_tensor(tensor_thread_t t, int idx) { return bfloat8(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, bfloat8 val) {}
METAL_FUNC bfloat16 load_tensor(tensor_thread_t t, int idx) { return bfloat16(); }
METAL_FUNC void store_tensor(tensor_thread_t t, int idx, bfloat16 val) {}

METAL_FUNC device void *get_data_pointer_tensor(device tensor_t t) { return nullptr; }
METAL_FUNC device void *data_handle_tensor(device tensor_thread_t t) { return nullptr; }
METAL_FUNC constant void *get_data_pointer_tensor(constant tensor_t t) { return nullptr; }
METAL_FUNC constant void *data_handle_tensor(constant tensor_thread_t t) { return nullptr; }
METAL_FUNC threadgroup void *get_data_pointer_tensor(threadgroup tensor_t t) { return nullptr; }
METAL_FUNC threadgroup void *data_handle_tensor(threadgroup tensor_thread_t t) { return nullptr; }

} // namespace metal
#endif