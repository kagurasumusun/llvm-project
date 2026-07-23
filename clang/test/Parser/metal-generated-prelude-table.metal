// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

uchar a;
ushort b;
uint c;
ulong d;
half e;
float4 f;
half4 h;
uint4 u;
metal::float4 mf;
metal::uint4 mu;
float4x4 matrix;
