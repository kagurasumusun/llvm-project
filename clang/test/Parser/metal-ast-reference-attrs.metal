// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

vertex void attr_vertex(device uint *out [[buffer(0)]],
                        uint vid [[vertex_id]],
                        uint iid [[instance_id]],
                        uint amp [[amplification_id]],
                        uint basev [[base_vertex]],
                        uint basei [[base_instance]]) {}

fragment void attr_fragment(bool front [[front_facing]],
                            float4 pos [[position]],
                            uint sid [[sample_id]],
                            uint mask [[sample_mask]],
                            uint pid [[primitive_id]],
                            float2 bc [[barycentric_coord]]) {}

kernel void attr_kernel(device uint *out [[buffer(0)]],
                        uint3 grid [[thread_position_in_grid]],
                        uint3 tgpos [[thread_position_in_threadgroup]],
                        uint3 tg [[threadgroup_position_in_grid]],
                        uint3 gridsz [[threads_per_grid]],
                        uint3 tgsz [[threads_per_threadgroup]],
                        uint idx [[thread_index_in_threadgroup]],
                        uint lane [[thread_index_in_simdgroup]],
                        uint simdidx [[simdgroup_index_in_threadgroup]],
                        uint simds [[simdgroups_per_threadgroup]]) {}
