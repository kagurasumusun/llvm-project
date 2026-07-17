#ifndef __METAL_SIMD_H__
#define __METAL_SIMD_H__

#if 1 // defined(__METAL__) || defined(__metal__)
namespace metal {

// SIMD group vote/ballot functions
inline bool simd_all(bool expr) { return expr; }
inline bool simd_any(bool expr) { return expr; }
inline unsigned long simd_ballot(bool expr) { return expr ? ~0UL : 0UL; }
inline unsigned long simd_active_threads_mask() { return ~0UL; }

// SIMD group broadcast & shuffle
template<typename T>
inline T simd_broadcast(T val, ushort broadcast_lane_id) { return val; }

template<typename T>
inline T simd_shuffle(T val, ushort target_lane_id) { return val; }

template<typename T>
inline T simd_shuffle_down(T val, ushort delta) { return val; }

template<typename T>
inline T simd_shuffle_up(T val, ushort delta) { return val; }

template<typename T>
inline T simd_shuffle_xor(T val, ushort mask) { return val; }

// SIMD group reductions & scans
template<typename T>
inline T simd_sum(T val) { return val; }

template<typename T>
inline T simd_min(T val) { return val; }

template<typename T>
inline T simd_max(T val) { return val; }

template<typename T>
inline T simd_prefix_exclusive_sum(T val) { return val; }

template<typename T>
inline T simd_prefix_inclusive_sum(T val) { return val; }

// Quad group functions
template<typename T>
inline T quad_broadcast(T val, ushort quad_lane_id) { return val; }

template<typename T>
inline T quad_shuffle(T val, ushort quad_lane_id) { return val; }

} // namespace metal
#endif
#endif // __METAL_SIMD_H__
