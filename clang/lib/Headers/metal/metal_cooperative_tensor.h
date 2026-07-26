// metal_cooperative_tensor — auto-generated
#ifndef _METAL_COOPERATIVE_TENSOR_H_
#define _METAL_COOPERATIVE_TENSOR_H_
#include <metal/metal_common>
namespace metal {

template <typename T, int... Dims>
struct cooperative_tensor {
  METAL_FUNC T get(int idx) const { return T(); }
  METAL_FUNC void set(int idx, T val) {}
  METAL_FUNC int get_rank() const { return sizeof...(Dims); }
  METAL_FUNC int get_capacity() const { return 1; }
  METAL_FUNC bool is_valid_element(int idx) const { return true; }
  METAL_FUNC auto get_iterator() const { return *this; }
};

template <> struct cooperative_tensor<float, 16> {
  METAL_FUNC float get(int idx) const { return float(); }
  METAL_FUNC void set(int idx, float val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 16; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 16; }
};

template <> struct cooperative_tensor<float, 32> {
  METAL_FUNC float get(int idx) const { return float(); }
  METAL_FUNC void set(int idx, float val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 32; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 32; }
};

template <> struct cooperative_tensor<float, 64> {
  METAL_FUNC float get(int idx) const { return float(); }
  METAL_FUNC void set(int idx, float val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 64; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 64; }
};

template <> struct cooperative_tensor<half, 16> {
  METAL_FUNC half get(int idx) const { return half(); }
  METAL_FUNC void set(int idx, half val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 16; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 16; }
};

template <> struct cooperative_tensor<half, 32> {
  METAL_FUNC half get(int idx) const { return half(); }
  METAL_FUNC void set(int idx, half val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 32; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 32; }
};

template <> struct cooperative_tensor<half, 64> {
  METAL_FUNC half get(int idx) const { return half(); }
  METAL_FUNC void set(int idx, half val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 64; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 64; }
};

template <> struct cooperative_tensor<int, 16> {
  METAL_FUNC int get(int idx) const { return int(); }
  METAL_FUNC void set(int idx, int val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 16; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 16; }
};

template <> struct cooperative_tensor<int, 32> {
  METAL_FUNC int get(int idx) const { return int(); }
  METAL_FUNC void set(int idx, int val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 32; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 32; }
};

template <> struct cooperative_tensor<int, 64> {
  METAL_FUNC int get(int idx) const { return int(); }
  METAL_FUNC void set(int idx, int val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 64; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 64; }
};

template <> struct cooperative_tensor<uint, 16> {
  METAL_FUNC uint get(int idx) const { return uint(); }
  METAL_FUNC void set(int idx, uint val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 16; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 16; }
};

template <> struct cooperative_tensor<uint, 32> {
  METAL_FUNC uint get(int idx) const { return uint(); }
  METAL_FUNC void set(int idx, uint val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 32; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 32; }
};

template <> struct cooperative_tensor<uint, 64> {
  METAL_FUNC uint get(int idx) const { return uint(); }
  METAL_FUNC void set(int idx, uint val) {}
  METAL_FUNC int get_rank() const { return 1; }
  METAL_FUNC int get_capacity() const { return 64; }
  METAL_FUNC bool is_valid_element(int idx) const { return idx < 64; }
};

} // namespace metal
#endif