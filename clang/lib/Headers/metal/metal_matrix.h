// metal_matrix — MSL matrix functions (auto-generated, Apple ABI compatible)
#ifndef _METAL_MATRIX_H_
#define _METAL_MATRIX_H_
#include <metal/metal_common>
#include <metal/metal_math>

namespace metal {

template <typename T, int Cols, int Rows>
struct matrix {
  vec<T, Rows> columns[Cols];

  METAL_FUNC matrix() = default;

  // Diagonal constructor
  template <typename U>
  METAL_FUNC explicit matrix(U diag)
  {
    for (int c = 0; c < Cols; ++c)
      for (int r = 0; r < Rows; ++r)
        columns[c][r] = (c == r) ? T(diag) : T(0);
  }

  // Column vectors constructor
  template <typename... U>
  METAL_FUNC matrix(U... cols)
  {
    _set_columns(0, cols...);
  }

  // Copy from different address space
  template <typename U>
  METAL_FUNC matrix(const thread matrix<U, Cols, Rows>& that)
  { for (int c = 0; c < Cols; ++c) columns[c] = vec<T, Rows>(that.columns[c]); }

  template <typename U>
  METAL_FUNC matrix(const device matrix<U, Cols, Rows>& that)
  { for (int c = 0; c < Cols; ++c) columns[c] = vec<T, Rows>(that.columns[c]); }

  template <typename U>
  METAL_FUNC matrix(const constant matrix<U, Cols, Rows>& that)
  { for (int c = 0; c < Cols; ++c) columns[c] = vec<T, Rows>(that.columns[c]); }

  template <typename U>
  METAL_FUNC matrix(const threadgroup matrix<U, Cols, Rows>& that)
  { for (int c = 0; c < Cols; ++c) columns[c] = vec<T, Rows>(that.columns[c]); }

  // Element access
  METAL_FUNC vec<T, Rows>& operator[](int c) { return columns[c]; }
  METAL_FUNC const vec<T, Rows>& operator[](int c) const { return columns[c]; }

  // Assignment from different address spaces
  METAL_FUNC matrix& operator=(const thread matrix& o)
  { for (int c = 0; c < Cols; ++c) columns[c] = o.columns[c]; return *this; }
  METAL_FUNC matrix& operator=(const device matrix& o)
  { for (int c = 0; c < Cols; ++c) columns[c] = o.columns[c]; return *this; }
  METAL_FUNC matrix& operator=(const constant matrix& o)
  { for (int c = 0; c < Cols; ++c) columns[c] = o.columns[c]; return *this; }
  METAL_FUNC matrix& operator=(const threadgroup matrix& o)
  { for (int c = 0; c < Cols; ++c) columns[c] = o.columns[c]; return *this; }

  // Arithmetic
  METAL_FUNC matrix& operator+=(const matrix& o)
  { for (int c = 0; c < Cols; ++c) columns[c] += o.columns[c]; return *this; }
  METAL_FUNC matrix& operator-=(const matrix& o)
  { for (int c = 0; c < Cols; ++c) columns[c] -= o.columns[c]; return *this; }
  METAL_FUNC matrix& operator*=(T s)
  { for (int c = 0; c < Cols; ++c) columns[c] *= s; return *this; }
  METAL_FUNC matrix& operator/=(T s)
  { for (int c = 0; c < Cols; ++c) columns[c] /= s; return *this; }

private:
  template <typename U, typename... Rest>
  METAL_FUNC void _set_columns(int i, U col, Rest... rest)
  { if (i < Cols) columns[i] = vec<T, Rows>(col); _set_columns(i + 1, rest...); }
  METAL_FUNC void _set_columns(int) {}
};

typedef matrix<float, 2, 2> float2x2;
typedef matrix<float, 3, 2> float2x3;
typedef matrix<float, 4, 2> float2x4;
typedef matrix<float, 2, 3> float3x2;
typedef matrix<float, 3, 3> float3x3;
typedef matrix<float, 4, 3> float3x4;
typedef matrix<float, 2, 4> float4x2;
typedef matrix<float, 3, 4> float4x3;
typedef matrix<float, 4, 4> float4x4;
typedef matrix<half, 2, 2> half2x2;
typedef matrix<half, 3, 2> half2x3;
typedef matrix<half, 4, 2> half2x4;
typedef matrix<half, 2, 3> half3x2;
typedef matrix<half, 3, 3> half3x3;
typedef matrix<half, 4, 3> half3x4;
typedef matrix<half, 2, 4> half4x2;
typedef matrix<half, 3, 4> half4x3;
typedef matrix<half, 4, 4> half4x4;

METAL_FUNC float2x2 operator+(float2x2 a, float2x2 b) {float2x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC float2x2 operator-(float2x2 a, float2x2 b) {float2x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC float2x2 operator-(float2x2 a) {float2x2 r; for(int i=0;i<2;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC float2x2 operator*(float2x2 a, float s) {float2x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC float2x2 operator*(float s, float2x2 a) {return a*s;}
METAL_FUNC float2x2 operator/(float2x2 a, float s) {float2x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC float2x2 operator/(float s, float2x2 a) {float2x2 r; for(int i=0;i<2;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC float2 operator*(float2x2 m, float2 v) {float2 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  return r;}
METAL_FUNC float2x2 transpose(float2x2 m) {float2x2 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  return r;}
METAL_FUNC float2x2 operator*(float2x2 a, float2x2 b) {float2x2 r;
  r.columns[0]=a.columns[0]*b.columns[0][0]+a.columns[1]*b.columns[0][1];
  r.columns[1]=a.columns[0]*b.columns[1][0]+a.columns[1]*b.columns[1][1];
  return r;}

METAL_FUNC float2x3 operator+(float2x3 a, float2x3 b) {float2x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC float2x3 operator-(float2x3 a, float2x3 b) {float2x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC float2x3 operator-(float2x3 a) {float2x3 r; for(int i=0;i<3;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC float2x3 operator*(float2x3 a, float s) {float2x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC float2x3 operator*(float s, float2x3 a) {return a*s;}
METAL_FUNC float2x3 operator/(float2x3 a, float s) {float2x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC float2x3 operator/(float s, float2x3 a) {float2x3 r; for(int i=0;i<3;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC float2 operator*(float2x3 m, float2 v) {float2 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  return r;}
METAL_FUNC float2x3 transpose(float2x3 m) {float2x3 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  return r;}

METAL_FUNC float2x4 operator+(float2x4 a, float2x4 b) {float2x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC float2x4 operator-(float2x4 a, float2x4 b) {float2x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC float2x4 operator-(float2x4 a) {float2x4 r; for(int i=0;i<4;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC float2x4 operator*(float2x4 a, float s) {float2x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC float2x4 operator*(float s, float2x4 a) {return a*s;}
METAL_FUNC float2x4 operator/(float2x4 a, float s) {float2x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC float2x4 operator/(float s, float2x4 a) {float2x4 r; for(int i=0;i<4;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC float2 operator*(float2x4 m, float2 v) {float2 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  r+=m.columns[3]*v.w;
  return r;}
METAL_FUNC float2x4 transpose(float2x4 m) {float2x4 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[0][3]=m.columns[3][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[1][3]=m.columns[3][1];
  return r;}

METAL_FUNC float3x2 operator+(float3x2 a, float3x2 b) {float3x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC float3x2 operator-(float3x2 a, float3x2 b) {float3x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC float3x2 operator-(float3x2 a) {float3x2 r; for(int i=0;i<2;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC float3x2 operator*(float3x2 a, float s) {float3x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC float3x2 operator*(float s, float3x2 a) {return a*s;}
METAL_FUNC float3x2 operator/(float3x2 a, float s) {float3x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC float3x2 operator/(float s, float3x2 a) {float3x2 r; for(int i=0;i<2;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC float3 operator*(float3x2 m, float3 v) {float3 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  return r;}
METAL_FUNC float3x2 transpose(float3x2 m) {float3x2 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  return r;}

METAL_FUNC float3x3 operator+(float3x3 a, float3x3 b) {float3x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC float3x3 operator-(float3x3 a, float3x3 b) {float3x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC float3x3 operator-(float3x3 a) {float3x3 r; for(int i=0;i<3;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC float3x3 operator*(float3x3 a, float s) {float3x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC float3x3 operator*(float s, float3x3 a) {return a*s;}
METAL_FUNC float3x3 operator/(float3x3 a, float s) {float3x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC float3x3 operator/(float s, float3x3 a) {float3x3 r; for(int i=0;i<3;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC float3 operator*(float3x3 m, float3 v) {float3 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  return r;}
METAL_FUNC float3x3 transpose(float3x3 m) {float3x3 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[2][2]=m.columns[2][2];
  return r;}
METAL_FUNC float3x3 operator*(float3x3 a, float3x3 b) {float3x3 r;
  r.columns[0]=a.columns[0]*b.columns[0][0]+a.columns[1]*b.columns[0][1]+a.columns[2]*b.columns[0][2];
  r.columns[1]=a.columns[0]*b.columns[1][0]+a.columns[1]*b.columns[1][1]+a.columns[2]*b.columns[1][2];
  r.columns[2]=a.columns[0]*b.columns[2][0]+a.columns[1]*b.columns[2][1]+a.columns[2]*b.columns[2][2];
  return r;}

METAL_FUNC float3x4 operator+(float3x4 a, float3x4 b) {float3x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC float3x4 operator-(float3x4 a, float3x4 b) {float3x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC float3x4 operator-(float3x4 a) {float3x4 r; for(int i=0;i<4;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC float3x4 operator*(float3x4 a, float s) {float3x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC float3x4 operator*(float s, float3x4 a) {return a*s;}
METAL_FUNC float3x4 operator/(float3x4 a, float s) {float3x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC float3x4 operator/(float s, float3x4 a) {float3x4 r; for(int i=0;i<4;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC float3 operator*(float3x4 m, float3 v) {float3 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  r+=m.columns[3]*v.w;
  return r;}
METAL_FUNC float3x4 transpose(float3x4 m) {float3x4 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[0][3]=m.columns[3][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[1][3]=m.columns[3][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[2][2]=m.columns[2][2];
  r.columns[2][3]=m.columns[3][2];
  return r;}

METAL_FUNC float4x2 operator+(float4x2 a, float4x2 b) {float4x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC float4x2 operator-(float4x2 a, float4x2 b) {float4x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC float4x2 operator-(float4x2 a) {float4x2 r; for(int i=0;i<2;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC float4x2 operator*(float4x2 a, float s) {float4x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC float4x2 operator*(float s, float4x2 a) {return a*s;}
METAL_FUNC float4x2 operator/(float4x2 a, float s) {float4x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC float4x2 operator/(float s, float4x2 a) {float4x2 r; for(int i=0;i<2;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC float4 operator*(float4x2 m, float4 v) {float4 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  return r;}
METAL_FUNC float4x2 transpose(float4x2 m) {float4x2 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[3][0]=m.columns[0][3];
  r.columns[3][1]=m.columns[1][3];
  return r;}

METAL_FUNC float4x3 operator+(float4x3 a, float4x3 b) {float4x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC float4x3 operator-(float4x3 a, float4x3 b) {float4x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC float4x3 operator-(float4x3 a) {float4x3 r; for(int i=0;i<3;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC float4x3 operator*(float4x3 a, float s) {float4x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC float4x3 operator*(float s, float4x3 a) {return a*s;}
METAL_FUNC float4x3 operator/(float4x3 a, float s) {float4x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC float4x3 operator/(float s, float4x3 a) {float4x3 r; for(int i=0;i<3;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC float4 operator*(float4x3 m, float4 v) {float4 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  return r;}
METAL_FUNC float4x3 transpose(float4x3 m) {float4x3 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[2][2]=m.columns[2][2];
  r.columns[3][0]=m.columns[0][3];
  r.columns[3][1]=m.columns[1][3];
  r.columns[3][2]=m.columns[2][3];
  return r;}

METAL_FUNC float4x4 operator+(float4x4 a, float4x4 b) {float4x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC float4x4 operator-(float4x4 a, float4x4 b) {float4x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC float4x4 operator-(float4x4 a) {float4x4 r; for(int i=0;i<4;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC float4x4 operator*(float4x4 a, float s) {float4x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC float4x4 operator*(float s, float4x4 a) {return a*s;}
METAL_FUNC float4x4 operator/(float4x4 a, float s) {float4x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC float4x4 operator/(float s, float4x4 a) {float4x4 r; for(int i=0;i<4;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC float4 operator*(float4x4 m, float4 v) {float4 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  r+=m.columns[3]*v.w;
  return r;}
METAL_FUNC float4x4 transpose(float4x4 m) {float4x4 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[0][3]=m.columns[3][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[1][3]=m.columns[3][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[2][2]=m.columns[2][2];
  r.columns[2][3]=m.columns[3][2];
  r.columns[3][0]=m.columns[0][3];
  r.columns[3][1]=m.columns[1][3];
  r.columns[3][2]=m.columns[2][3];
  r.columns[3][3]=m.columns[3][3];
  return r;}
METAL_FUNC float4x4 operator*(float4x4 a, float4x4 b) {float4x4 r;
  r.columns[0]=a.columns[0]*b.columns[0][0]+a.columns[1]*b.columns[0][1]+a.columns[2]*b.columns[0][2]+a.columns[3]*b.columns[0][3];
  r.columns[1]=a.columns[0]*b.columns[1][0]+a.columns[1]*b.columns[1][1]+a.columns[2]*b.columns[1][2]+a.columns[3]*b.columns[1][3];
  r.columns[2]=a.columns[0]*b.columns[2][0]+a.columns[1]*b.columns[2][1]+a.columns[2]*b.columns[2][2]+a.columns[3]*b.columns[2][3];
  r.columns[3]=a.columns[0]*b.columns[3][0]+a.columns[1]*b.columns[3][1]+a.columns[2]*b.columns[3][2]+a.columns[3]*b.columns[3][3];
  return r;}

METAL_FUNC half2x2 operator+(half2x2 a, half2x2 b) {half2x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC half2x2 operator-(half2x2 a, half2x2 b) {half2x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC half2x2 operator-(half2x2 a) {half2x2 r; for(int i=0;i<2;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC half2x2 operator*(half2x2 a, half s) {half2x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC half2x2 operator*(half s, half2x2 a) {return a*s;}
METAL_FUNC half2x2 operator/(half2x2 a, half s) {half2x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC half2x2 operator/(half s, half2x2 a) {half2x2 r; for(int i=0;i<2;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC half2 operator*(half2x2 m, half2 v) {half2 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  return r;}
METAL_FUNC half2x2 transpose(half2x2 m) {half2x2 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  return r;}
METAL_FUNC half2x2 operator*(half2x2 a, half2x2 b) {half2x2 r;
  r.columns[0]=a.columns[0]*b.columns[0][0]+a.columns[1]*b.columns[0][1];
  r.columns[1]=a.columns[0]*b.columns[1][0]+a.columns[1]*b.columns[1][1];
  return r;}

METAL_FUNC half2x3 operator+(half2x3 a, half2x3 b) {half2x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC half2x3 operator-(half2x3 a, half2x3 b) {half2x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC half2x3 operator-(half2x3 a) {half2x3 r; for(int i=0;i<3;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC half2x3 operator*(half2x3 a, half s) {half2x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC half2x3 operator*(half s, half2x3 a) {return a*s;}
METAL_FUNC half2x3 operator/(half2x3 a, half s) {half2x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC half2x3 operator/(half s, half2x3 a) {half2x3 r; for(int i=0;i<3;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC half2 operator*(half2x3 m, half2 v) {half2 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  return r;}
METAL_FUNC half2x3 transpose(half2x3 m) {half2x3 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  return r;}

METAL_FUNC half2x4 operator+(half2x4 a, half2x4 b) {half2x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC half2x4 operator-(half2x4 a, half2x4 b) {half2x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC half2x4 operator-(half2x4 a) {half2x4 r; for(int i=0;i<4;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC half2x4 operator*(half2x4 a, half s) {half2x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC half2x4 operator*(half s, half2x4 a) {return a*s;}
METAL_FUNC half2x4 operator/(half2x4 a, half s) {half2x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC half2x4 operator/(half s, half2x4 a) {half2x4 r; for(int i=0;i<4;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC half2 operator*(half2x4 m, half2 v) {half2 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  r+=m.columns[3]*v.w;
  return r;}
METAL_FUNC half2x4 transpose(half2x4 m) {half2x4 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[0][3]=m.columns[3][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[1][3]=m.columns[3][1];
  return r;}

METAL_FUNC half3x2 operator+(half3x2 a, half3x2 b) {half3x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC half3x2 operator-(half3x2 a, half3x2 b) {half3x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC half3x2 operator-(half3x2 a) {half3x2 r; for(int i=0;i<2;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC half3x2 operator*(half3x2 a, half s) {half3x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC half3x2 operator*(half s, half3x2 a) {return a*s;}
METAL_FUNC half3x2 operator/(half3x2 a, half s) {half3x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC half3x2 operator/(half s, half3x2 a) {half3x2 r; for(int i=0;i<2;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC half3 operator*(half3x2 m, half3 v) {half3 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  return r;}
METAL_FUNC half3x2 transpose(half3x2 m) {half3x2 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  return r;}

METAL_FUNC half3x3 operator+(half3x3 a, half3x3 b) {half3x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC half3x3 operator-(half3x3 a, half3x3 b) {half3x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC half3x3 operator-(half3x3 a) {half3x3 r; for(int i=0;i<3;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC half3x3 operator*(half3x3 a, half s) {half3x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC half3x3 operator*(half s, half3x3 a) {return a*s;}
METAL_FUNC half3x3 operator/(half3x3 a, half s) {half3x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC half3x3 operator/(half s, half3x3 a) {half3x3 r; for(int i=0;i<3;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC half3 operator*(half3x3 m, half3 v) {half3 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  return r;}
METAL_FUNC half3x3 transpose(half3x3 m) {half3x3 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[2][2]=m.columns[2][2];
  return r;}
METAL_FUNC half3x3 operator*(half3x3 a, half3x3 b) {half3x3 r;
  r.columns[0]=a.columns[0]*b.columns[0][0]+a.columns[1]*b.columns[0][1]+a.columns[2]*b.columns[0][2];
  r.columns[1]=a.columns[0]*b.columns[1][0]+a.columns[1]*b.columns[1][1]+a.columns[2]*b.columns[1][2];
  r.columns[2]=a.columns[0]*b.columns[2][0]+a.columns[1]*b.columns[2][1]+a.columns[2]*b.columns[2][2];
  return r;}

METAL_FUNC half3x4 operator+(half3x4 a, half3x4 b) {half3x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC half3x4 operator-(half3x4 a, half3x4 b) {half3x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC half3x4 operator-(half3x4 a) {half3x4 r; for(int i=0;i<4;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC half3x4 operator*(half3x4 a, half s) {half3x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC half3x4 operator*(half s, half3x4 a) {return a*s;}
METAL_FUNC half3x4 operator/(half3x4 a, half s) {half3x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC half3x4 operator/(half s, half3x4 a) {half3x4 r; for(int i=0;i<4;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC half3 operator*(half3x4 m, half3 v) {half3 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  r+=m.columns[3]*v.w;
  return r;}
METAL_FUNC half3x4 transpose(half3x4 m) {half3x4 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[0][3]=m.columns[3][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[1][3]=m.columns[3][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[2][2]=m.columns[2][2];
  r.columns[2][3]=m.columns[3][2];
  return r;}

METAL_FUNC half4x2 operator+(half4x2 a, half4x2 b) {half4x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC half4x2 operator-(half4x2 a, half4x2 b) {half4x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC half4x2 operator-(half4x2 a) {half4x2 r; for(int i=0;i<2;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC half4x2 operator*(half4x2 a, half s) {half4x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC half4x2 operator*(half s, half4x2 a) {return a*s;}
METAL_FUNC half4x2 operator/(half4x2 a, half s) {half4x2 r; for(int i=0;i<2;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC half4x2 operator/(half s, half4x2 a) {half4x2 r; for(int i=0;i<2;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC half4 operator*(half4x2 m, half4 v) {half4 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  return r;}
METAL_FUNC half4x2 transpose(half4x2 m) {half4x2 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[3][0]=m.columns[0][3];
  r.columns[3][1]=m.columns[1][3];
  return r;}

METAL_FUNC half4x3 operator+(half4x3 a, half4x3 b) {half4x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC half4x3 operator-(half4x3 a, half4x3 b) {half4x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC half4x3 operator-(half4x3 a) {half4x3 r; for(int i=0;i<3;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC half4x3 operator*(half4x3 a, half s) {half4x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC half4x3 operator*(half s, half4x3 a) {return a*s;}
METAL_FUNC half4x3 operator/(half4x3 a, half s) {half4x3 r; for(int i=0;i<3;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC half4x3 operator/(half s, half4x3 a) {half4x3 r; for(int i=0;i<3;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC half4 operator*(half4x3 m, half4 v) {half4 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  return r;}
METAL_FUNC half4x3 transpose(half4x3 m) {half4x3 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[2][2]=m.columns[2][2];
  r.columns[3][0]=m.columns[0][3];
  r.columns[3][1]=m.columns[1][3];
  r.columns[3][2]=m.columns[2][3];
  return r;}

METAL_FUNC half4x4 operator+(half4x4 a, half4x4 b) {half4x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]+b.columns[i]; return r;}
METAL_FUNC half4x4 operator-(half4x4 a, half4x4 b) {half4x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]-b.columns[i]; return r;}
METAL_FUNC half4x4 operator-(half4x4 a) {half4x4 r; for(int i=0;i<4;++i) r.columns[i]=-a.columns[i]; return r;}
METAL_FUNC half4x4 operator*(half4x4 a, half s) {half4x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]*s; return r;}
METAL_FUNC half4x4 operator*(half s, half4x4 a) {return a*s;}
METAL_FUNC half4x4 operator/(half4x4 a, half s) {half4x4 r; for(int i=0;i<4;++i) r.columns[i]=a.columns[i]/s; return r;}
METAL_FUNC half4x4 operator/(half s, half4x4 a) {half4x4 r; for(int i=0;i<4;++i) r.columns[i]=s/a.columns[i]; return r;}
METAL_FUNC half4 operator*(half4x4 m, half4 v) {half4 r=m.columns[0]*v.x;
  r+=m.columns[1]*v.y;
  r+=m.columns[2]*v.z;
  r+=m.columns[3]*v.w;
  return r;}
METAL_FUNC half4x4 transpose(half4x4 m) {half4x4 r;
  r.columns[0][0]=m.columns[0][0];
  r.columns[0][1]=m.columns[1][0];
  r.columns[0][2]=m.columns[2][0];
  r.columns[0][3]=m.columns[3][0];
  r.columns[1][0]=m.columns[0][1];
  r.columns[1][1]=m.columns[1][1];
  r.columns[1][2]=m.columns[2][1];
  r.columns[1][3]=m.columns[3][1];
  r.columns[2][0]=m.columns[0][2];
  r.columns[2][1]=m.columns[1][2];
  r.columns[2][2]=m.columns[2][2];
  r.columns[2][3]=m.columns[3][2];
  r.columns[3][0]=m.columns[0][3];
  r.columns[3][1]=m.columns[1][3];
  r.columns[3][2]=m.columns[2][3];
  r.columns[3][3]=m.columns[3][3];
  return r;}
METAL_FUNC half4x4 operator*(half4x4 a, half4x4 b) {half4x4 r;
  r.columns[0]=a.columns[0]*b.columns[0][0]+a.columns[1]*b.columns[0][1]+a.columns[2]*b.columns[0][2]+a.columns[3]*b.columns[0][3];
  r.columns[1]=a.columns[0]*b.columns[1][0]+a.columns[1]*b.columns[1][1]+a.columns[2]*b.columns[1][2]+a.columns[3]*b.columns[1][3];
  r.columns[2]=a.columns[0]*b.columns[2][0]+a.columns[1]*b.columns[2][1]+a.columns[2]*b.columns[2][2]+a.columns[3]*b.columns[2][3];
  r.columns[3]=a.columns[0]*b.columns[3][0]+a.columns[1]*b.columns[3][1]+a.columns[2]*b.columns[3][2]+a.columns[3]*b.columns[3][3];
  return r;}

METAL_FUNC float determinant(float2x2 m)
{ return m.columns[0][0]*m.columns[1][1]-m.columns[1][0]*m.columns[0][1]; }
METAL_FUNC float determinant(float3x3 m)
{ return m.columns[0][0]*(m.columns[1][1]*m.columns[2][2]-m.columns[2][1]*m.columns[1][2])
       -m.columns[1][0]*(m.columns[0][1]*m.columns[2][2]-m.columns[2][1]*m.columns[0][2])
       +m.columns[2][0]*(m.columns[0][1]*m.columns[1][2]-m.columns[1][1]*m.columns[0][2]); }
METAL_FUNC float determinant(float4x4 m)
{ float s0=m.columns[0][0]*m.columns[1][1]-m.columns[1][0]*m.columns[0][1];
  float s1=m.columns[0][0]*m.columns[1][2]-m.columns[1][0]*m.columns[0][2];
  float s2=m.columns[0][0]*m.columns[1][3]-m.columns[1][0]*m.columns[0][3];
  float s3=m.columns[0][1]*m.columns[1][2]-m.columns[1][1]*m.columns[0][2];
  float s4=m.columns[0][1]*m.columns[1][3]-m.columns[1][1]*m.columns[0][3];
  float s5=m.columns[0][2]*m.columns[1][3]-m.columns[1][2]*m.columns[0][3];
  float c5=m.columns[2][2]*m.columns[3][3]-m.columns[3][2]*m.columns[2][3];
  float c4=m.columns[2][1]*m.columns[3][3]-m.columns[3][1]*m.columns[2][3];
  float c3=m.columns[2][1]*m.columns[3][2]-m.columns[3][1]*m.columns[2][2];
  float c2=m.columns[2][0]*m.columns[3][3]-m.columns[3][0]*m.columns[2][3];
  float c1=m.columns[2][0]*m.columns[3][2]-m.columns[3][0]*m.columns[2][2];
  float c0=m.columns[2][0]*m.columns[3][1]-m.columns[3][0]*m.columns[2][1];
  return s0*c5-s1*c4+s2*c3+s3*c2-s4*c1+s5*c0; }
METAL_FUNC half determinant(half2x2 m)
{ return m.columns[0][0]*m.columns[1][1]-m.columns[1][0]*m.columns[0][1]; }
METAL_FUNC half determinant(half3x3 m)
{ return m.columns[0][0]*(m.columns[1][1]*m.columns[2][2]-m.columns[2][1]*m.columns[1][2])
       -m.columns[1][0]*(m.columns[0][1]*m.columns[2][2]-m.columns[2][1]*m.columns[0][2])
       +m.columns[2][0]*(m.columns[0][1]*m.columns[1][2]-m.columns[1][1]*m.columns[0][2]); }
METAL_FUNC half determinant(half4x4 m)
{ half s0=m.columns[0][0]*m.columns[1][1]-m.columns[1][0]*m.columns[0][1];
  half s1=m.columns[0][0]*m.columns[1][2]-m.columns[1][0]*m.columns[0][2];
  half s2=m.columns[0][0]*m.columns[1][3]-m.columns[1][0]*m.columns[0][3];
  half s3=m.columns[0][1]*m.columns[1][2]-m.columns[1][1]*m.columns[0][2];
  half s4=m.columns[0][1]*m.columns[1][3]-m.columns[1][1]*m.columns[0][3];
  half s5=m.columns[0][2]*m.columns[1][3]-m.columns[1][2]*m.columns[0][3];
  half c5=m.columns[2][2]*m.columns[3][3]-m.columns[3][2]*m.columns[2][3];
  half c4=m.columns[2][1]*m.columns[3][3]-m.columns[3][1]*m.columns[2][3];
  half c3=m.columns[2][1]*m.columns[3][2]-m.columns[3][1]*m.columns[2][2];
  half c2=m.columns[2][0]*m.columns[3][3]-m.columns[3][0]*m.columns[2][3];
  half c1=m.columns[2][0]*m.columns[3][2]-m.columns[3][0]*m.columns[2][2];
  half c0=m.columns[2][0]*m.columns[3][1]-m.columns[3][0]*m.columns[2][1];
  return s0*c5-s1*c4+s2*c3+s3*c2-s4*c1+s5*c0; }

} // namespace metal
#endif