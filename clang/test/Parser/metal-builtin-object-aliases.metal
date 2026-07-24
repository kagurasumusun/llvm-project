// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

texture2d tex2d;
depth2d depth;
sampler s;
visible_function_table vft;
instance_acceleration_structure ias;
primitive_acceleration_structure pas;
intersection_query iq;
tensor t;

// User-facing namespace-qualified object wrappers are provided by Apple
// metal_stdlib, not by Clang's lightweight bootstrap prelude.
