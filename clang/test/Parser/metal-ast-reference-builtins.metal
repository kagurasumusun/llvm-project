// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only %s

__metal_texture_2d_t *texture_obj;
__metal_sampler_t *sampler_obj;
__metal_visible_function_table_t *vft_obj;
__metal_instance_acceleration_structure_t *ias_obj;
__metal_tensor_t *tensor_obj;
