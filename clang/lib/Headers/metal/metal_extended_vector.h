#ifndef _METAL_EXTENDED_VECTOR_H_
#define _METAL_EXTENDED_VECTOR_H_
#include <metal/metal_common>
namespace metal {
template <typename T, int N> struct vec { T data[N]; };
typedef vec<float,8>  float8;
typedef vec<float,16> float16;
typedef vec<half,8>   half8;
typedef vec<half,16>  half16;
typedef vec<int,8>    int8;
typedef vec<int,16>   int16;
typedef vec<uint,8>   uint8;
typedef vec<uint,16>  uint16;
typedef vec<short,8>  short8;
typedef vec<short,16> short16;
typedef vec<ushort,8> ushort8;
typedef vec<ushort,16>ushort16;
typedef vec<char,8>   char8;
typedef vec<char,16>  char16;
typedef vec<uchar,8>  uchar8;
typedef vec<uchar,16> uchar16;
} // namespace metal
#endif
