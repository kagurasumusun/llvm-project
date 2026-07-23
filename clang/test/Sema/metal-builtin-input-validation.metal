// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only -verify %s

struct In { int x [[attribute(0)]]; };

kernel void good_kernel(uint3 grid [[thread_position_in_grid]],
                        uint tid [[thread_index_in_threadgroup]]) {}
vertex void good_vertex(In in [[stage_in]], uint vid [[vertex_id]]) {}
fragment void good_fragment(In in [[stage_in]], bool f [[front_facing]],
                            float4 p [[position]], uint sid [[sample_id]]) {}

fragment void bad_fragment_type(int front [[front_facing]]) {} // expected-error {{'front_facing' attribute requires parameter type bool}}
fragment void bad_position_type(uint p [[position]]) {} // expected-error {{'position' attribute requires parameter type float4}}
vertex void bad_stage_type(uint in [[stage_in]]) {} // expected-error {{'stage_in' attribute requires parameter type a record type}}
