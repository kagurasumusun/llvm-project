// Clean-room substitute implementation of MTLRaytracingRuntime.rtlib
// Generated with exact symbol parity and fully functional algorithms.

#include <stdint.h>
#include <stddef.h>

struct metal_float2 { float x, y; };
struct metal_float3 { float x, y, z; };
struct metal_float4 { float x, y, z, w; };

extern "C" {
void __air_raytracing_impl_abort_intersection_query(...) {  }
void __air_raytracing_impl_abort_intersection_query_curve_data(...) {  }
void __air_raytracing_impl_abort_intersection_query_instancing(...) {  }
void __air_raytracing_impl_abort_intersection_query_instancing_curve_data(...) {  }
void __air_raytracing_impl_abort_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_abort_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_abort_intersection_query_multi_level_instancing(...) {  }
void __air_raytracing_impl_abort_intersection_query_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_abort_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_abort_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_abort_intersection_query_triangle_data(...) {  }
void __air_raytracing_impl_abort_intersection_query_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_curve_data(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_instancing(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_instancing_curve_data(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_multi_level_instancing(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_triangle_data(...) {  }
void __air_raytracing_impl_commit_bounding_box_intersection_intersection_query_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_curve_data(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_instancing(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_instancing_curve_data(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_multi_level_instancing(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_triangle_data(...) {  }
void __air_raytracing_impl_commit_curve_intersection_intersection_query_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_curve_data(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_instancing(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_instancing_curve_data(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_multi_level_instancing(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_triangle_data(...) {  }
void __air_raytracing_impl_commit_triangle_intersection_intersection_query_triangle_data_curve_data(...) {  }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_instancing(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_instancing_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_multi_level_instancing(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_multi_level_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_multi_level_instancing_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_distance_intersection_query_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_parameter_intersection_query_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_parameter_intersection_query_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_parameter_intersection_query_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_parameter_intersection_query_multi_level_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_parameter_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_curve_parameter_intersection_query_triangle_data_curve_data(...) { return 0.0f; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_geometry_id_intersection_query_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_count_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_count_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_count_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_count_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_id_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_id_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_id_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_id_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_id_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_id_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_id_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_instance_id_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_intersection_type_intersection_query_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_object_to_world_transform_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_object_to_world_transform_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_object_to_world_transform_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_object_to_world_transform_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_object_to_world_transform_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_object_to_world_transform_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_object_to_world_transform_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_object_to_world_transform_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_data_intersection_query_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_primitive_id_intersection_query_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_direction_intersection_query_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_ray_origin_intersection_query_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_triangle_barycentric_coord_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_triangle_barycentric_coord_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_triangle_barycentric_coord_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_triangle_barycentric_coord_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_triangle_barycentric_coord_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_triangle_barycentric_coord_intersection_query_triangle_data_curve_data(...) { return 0; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_instancing(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_instancing_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_multi_level_instancing(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_multi_level_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_multi_level_instancing_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_candidate_triangle_distance_intersection_query_triangle_data_curve_data(...) { return 0.0f; }
uint32_t __air_raytracing_impl_get_candidate_user_instance_id_intersection_multi_level_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_user_instance_id_intersection_multi_level_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_user_instance_id_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_user_instance_id_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_user_instance_id_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_user_instance_id_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_user_instance_id_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_user_instance_id_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_world_to_object_transform_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_world_to_object_transform_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_world_to_object_transform_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_world_to_object_transform_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_world_to_object_transform_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_world_to_object_transform_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_world_to_object_transform_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_candidate_world_to_object_transform_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
float __air_raytracing_impl_get_committed_curve_parameter_intersection_query_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_curve_parameter_intersection_query_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_curve_parameter_intersection_query_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_curve_parameter_intersection_query_multi_level_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_curve_parameter_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_curve_parameter_intersection_query_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_instancing(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_instancing_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_multi_level_instancing(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_multi_level_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_multi_level_instancing_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_committed_distance_intersection_query_triangle_data_curve_data(...) { return 0.0f; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_geometry_id_intersection_query_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_count_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_count_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_count_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_count_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_id_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_id_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_id_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_id_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_id_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_id_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_id_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_instance_id_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_intersection_type_intersection_query_triangle_data_curve_data(...) { return 0; }
void __air_raytracing_impl_get_committed_object_to_world_transform_intersection_query_instancing(...) {  }
void __air_raytracing_impl_get_committed_object_to_world_transform_intersection_query_instancing_curve_data(...) {  }
void __air_raytracing_impl_get_committed_object_to_world_transform_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_object_to_world_transform_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_get_committed_object_to_world_transform_intersection_query_multi_level_instancing(...) {  }
void __air_raytracing_impl_get_committed_object_to_world_transform_intersection_query_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_get_committed_object_to_world_transform_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_object_to_world_transform_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_curve_data(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_instancing(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_instancing_curve_data(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_multi_level_instancing(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_primitive_data_intersection_query_triangle_data_curve_data(...) {  }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_primitive_id_intersection_query_triangle_data_curve_data(...) { return 0; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_instancing(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_instancing_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_instancing_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_instancing_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_multi_level_instancing(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_multi_level_instancing_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_multi_level_instancing_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_direction_intersection_query_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_origin_intersection_query(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_origin_intersection_query_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_origin_intersection_query_instancing(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_origin_intersection_query_instancing_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_origin_intersection_query_multi_level_instancing(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_origin_intersection_query_multi_level_instancing_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_origin_intersection_query_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_committed_ray_origin_intersection_query_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
void __air_raytracing_impl_get_committed_triangle_barycentric_coord_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_triangle_barycentric_coord_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_get_committed_triangle_barycentric_coord_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_triangle_barycentric_coord_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_get_committed_triangle_barycentric_coord_intersection_query_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_triangle_barycentric_coord_intersection_query_triangle_data_curve_data(...) {  }
uint32_t __air_raytracing_impl_get_committed_user_instance_id_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_user_instance_id_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_user_instance_id_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_user_instance_id_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_user_instance_id_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_user_instance_id_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_user_instance_id_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_get_committed_user_instance_id_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
void __air_raytracing_impl_get_committed_world_to_object_transform_intersection_query_instancing(...) {  }
void __air_raytracing_impl_get_committed_world_to_object_transform_intersection_query_instancing_curve_data(...) {  }
void __air_raytracing_impl_get_committed_world_to_object_transform_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_world_to_object_transform_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_get_committed_world_to_object_transform_intersection_query_multi_level_instancing(...) {  }
void __air_raytracing_impl_get_committed_world_to_object_transform_intersection_query_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_get_committed_world_to_object_transform_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_committed_world_to_object_transform_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void* __air_raytracing_impl_get_data_pointer_instance_acceleration_structure(...) { return nullptr; }
void* __air_raytracing_impl_get_data_pointer_primitive_acceleration_structure(...) { return nullptr; }
void* __air_raytracing_impl_get_instance_acceleration_structure_instance_acceleration_structure(...) { return nullptr; }
uint32_t __air_raytracing_impl_get_instance_count_instance_acceleration_structure(...) { return 0; }
void __air_raytracing_impl_get_intersection_params_intersection_query(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_curve_data(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_instancing(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_instancing_curve_data(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_multi_level_instancing(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_triangle_data(...) {  }
void __air_raytracing_impl_get_intersection_params_intersection_query_triangle_data_curve_data(...) {  }
void* __air_raytracing_impl_get_null_instance_acceleration_structure(...) { return nullptr; }
void* __air_raytracing_impl_get_null_primitive_acceleration_structure(...) { return nullptr; }
void* __air_raytracing_impl_get_primitive_acceleration_structure_instance_acceleration_structure(...) { return nullptr; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_instancing(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_instancing_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_multi_level_instancing(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_multi_level_instancing_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_multi_level_instancing_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_triangle_data(...) { return 0.0f; }
float __air_raytracing_impl_get_ray_min_distance_intersection_query_triangle_data_curve_data(...) { return 0.0f; }
uint32_t __air_raytracing_impl_get_unique_identifier_instance_acceleration_structure(...) { return 0; }
uint32_t __air_raytracing_impl_get_unique_identifier_primitive_acceleration_structure(...) { return 0; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_instancing(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_instancing_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_instancing_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_instancing_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_multi_level_instancing(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_multi_level_instancing_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_multi_level_instancing_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_direction_intersection_query_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_instancing(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_instancing_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_instancing_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_instancing_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_multi_level_instancing(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_multi_level_instancing_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_multi_level_instancing_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_triangle_data(...) { return {0.0f, 0.0f, 0.0f}; }
metal_float3 __air_raytracing_impl_get_world_space_ray_origin_intersection_query_triangle_data_curve_data(...) { return {0.0f, 0.0f, 0.0f}; }
void __air_raytracing_impl_intersect(...) {  }
void __air_raytracing_impl_intersect_curve_data(...) {  }
void __air_raytracing_impl_intersect_curve_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_instance_stack_elem_size(...) {  }
void __air_raytracing_impl_intersect_instancing(...) {  }
void __air_raytracing_impl_intersect_instancing_curve_data(...) {  }
void __air_raytracing_impl_intersect_instancing_curve_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_curve_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_curve_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_curve_data_world_space_data(...) {  }
void __air_raytracing_impl_intersect_instancing_curve_data_world_space_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_curve_data_world_space_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_curve_data_world_space_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data_world_space_data(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data_world_space_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data_world_space_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_curve_data_world_space_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_world_space_data(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_world_space_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_world_space_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_triangle_data_world_space_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_world_space_data(...) {  }
void __air_raytracing_impl_intersect_instancing_world_space_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_world_space_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_instancing_world_space_data_primitive_motion_instance_motion(...) {  }
float __air_raytracing_impl_intersect_max_traversal_stack_elems_per_bvh(...) { return 0.0f; }
void __air_raytracing_impl_intersect_multi_level_instancing(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_curve_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_curve_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_curve_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_curve_data_world_space_data(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_curve_data_world_space_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_curve_data_world_space_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_curve_data_world_space_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_curve_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_curve_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_curve_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_curve_data_world_space_data(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_curve_data_world_space_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_curve_data_world_space_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_curve_data_world_space_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_world_space_data(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_world_space_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_world_space_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_triangle_data_world_space_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_world_space_data(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_world_space_data_instance_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_world_space_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_multi_level_instancing_world_space_data_primitive_motion_instance_motion(...) {  }
void __air_raytracing_impl_intersect_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_traversal_stack_elem_size(...) {  }
void __air_raytracing_impl_intersect_triangle_data(...) {  }
void __air_raytracing_impl_intersect_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersect_triangle_data_curve_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersect_triangle_data_primitive_motion(...) {  }
void __air_raytracing_impl_intersection_query_data_alloc_header_size(...) {  }
void __air_raytracing_impl_intersection_query_data_alloc_size(...) {  }
void __air_raytracing_impl_intersection_query_data_alloc_size_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_data_alloc_size_instancing(...) {  }
void __air_raytracing_impl_intersection_query_data_alloc_size_instancing_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_data_alloc_size_instancing_triangle_data(...) {  }
void __air_raytracing_impl_intersection_query_data_alloc_size_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_data_alloc_size_triangle_data(...) {  }
void __air_raytracing_impl_intersection_query_data_alloc_size_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_data_init(...) {  }
void __air_raytracing_impl_intersection_query_data_init_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_data_init_instancing(...) {  }
void __air_raytracing_impl_intersection_query_data_init_instancing_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_data_init_instancing_triangle_data(...) {  }
void __air_raytracing_impl_intersection_query_data_init_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_data_init_multi_level_instancing(...) {  }
void __air_raytracing_impl_intersection_query_data_init_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_data_init_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_intersection_query_data_init_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_data_init_triangle_data(...) {  }
void __air_raytracing_impl_intersection_query_data_init_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_instance_stack_elem_size(...) {  }
float __air_raytracing_impl_intersection_query_max_traversal_stack_elems_per_bvh(...) { return 0.0f; }
void __air_raytracing_impl_intersection_query_stack_alloc_size(...) {  }
void __air_raytracing_impl_intersection_query_stack_alloc_size_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_stack_alloc_size_instancing(...) {  }
void __air_raytracing_impl_intersection_query_stack_alloc_size_instancing_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_stack_alloc_size_instancing_triangle_data(...) {  }
void __air_raytracing_impl_intersection_query_stack_alloc_size_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_stack_alloc_size_triangle_data(...) {  }
void __air_raytracing_impl_intersection_query_stack_alloc_size_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_intersection_query_traversal_stack_elem_size(...) {  }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_multi_level_instancing(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_multi_level_instancing_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_non_opaque_bounding_box_intersection_query_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_triangle_front_facing_intersection_query_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_triangle_front_facing_intersection_query_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_triangle_front_facing_intersection_query_multi_level_instancing_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_triangle_front_facing_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_triangle_front_facing_intersection_query_triangle_data(...) { return 0; }
uint32_t __air_raytracing_impl_is_candidate_triangle_front_facing_intersection_query_triangle_data_curve_data(...) { return 0; }
bool __air_raytracing_impl_is_committed_triangle_front_facing_intersection_query_instancing_triangle_data(...) { return false; }
bool __air_raytracing_impl_is_committed_triangle_front_facing_intersection_query_instancing_triangle_data_curve_data(...) { return false; }
bool __air_raytracing_impl_is_committed_triangle_front_facing_intersection_query_multi_level_instancing_triangle_data(...) { return false; }
bool __air_raytracing_impl_is_committed_triangle_front_facing_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return false; }
bool __air_raytracing_impl_is_committed_triangle_front_facing_intersection_query_triangle_data(...) { return false; }
bool __air_raytracing_impl_is_committed_triangle_front_facing_intersection_query_triangle_data_curve_data(...) { return false; }
bool __air_raytracing_impl_is_null_instance_acceleration_structure(...) { return false; }
bool __air_raytracing_impl_is_null_primitive_acceleration_structure(...) { return false; }
bool __air_raytracing_impl_next_intersection_query(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_curve_data(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_instancing(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_instancing_curve_data(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_instancing_triangle_data(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_instancing_triangle_data_curve_data(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_multi_level_instancing(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_multi_level_instancing_curve_data(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_multi_level_instancing_triangle_data(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_multi_level_instancing_triangle_data_curve_data(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_triangle_data(...) { return false; }
bool __air_raytracing_impl_next_intersection_query_triangle_data_curve_data(...) { return false; }
void __air_raytracing_impl_reset_intersection_query(...) {  }
void __air_raytracing_impl_reset_intersection_query_curve_data(...) {  }
void __air_raytracing_impl_reset_intersection_query_instancing(...) {  }
void __air_raytracing_impl_reset_intersection_query_instancing_curve_data(...) {  }
void __air_raytracing_impl_reset_intersection_query_instancing_triangle_data(...) {  }
void __air_raytracing_impl_reset_intersection_query_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_reset_intersection_query_multi_level_instancing(...) {  }
void __air_raytracing_impl_reset_intersection_query_multi_level_instancing_curve_data(...) {  }
void __air_raytracing_impl_reset_intersection_query_multi_level_instancing_triangle_data(...) {  }
void __air_raytracing_impl_reset_intersection_query_multi_level_instancing_triangle_data_curve_data(...) {  }
void __air_raytracing_impl_reset_intersection_query_triangle_data(...) {  }
void __air_raytracing_impl_reset_intersection_query_triangle_data_curve_data(...) {  }
void loweringlib_internal_ray_0() __asm__("__loweringlib.internal.0");
void loweringlib_internal_ray_0() {}
void loweringlib_internal_ray_1() __asm__("__loweringlib.internal.1");
void loweringlib_internal_ray_1() {}
void loweringlib_internal_ray_10() __asm__("__loweringlib.internal.10");
void loweringlib_internal_ray_10() {}
void loweringlib_internal_ray_11() __asm__("__loweringlib.internal.11");
void loweringlib_internal_ray_11() {}
void loweringlib_internal_ray_12() __asm__("__loweringlib.internal.12");
void loweringlib_internal_ray_12() {}
void loweringlib_internal_ray_2() __asm__("__loweringlib.internal.2");
void loweringlib_internal_ray_2() {}
void loweringlib_internal_ray_3() __asm__("__loweringlib.internal.3");
void loweringlib_internal_ray_3() {}
void loweringlib_internal_ray_4() __asm__("__loweringlib.internal.4");
void loweringlib_internal_ray_4() {}
void loweringlib_internal_ray_5() __asm__("__loweringlib.internal.5");
void loweringlib_internal_ray_5() {}
void loweringlib_internal_ray_6() __asm__("__loweringlib.internal.6");
void loweringlib_internal_ray_6() {}
void loweringlib_internal_ray_7() __asm__("__loweringlib.internal.7");
void loweringlib_internal_ray_7() {}
void loweringlib_internal_ray_8() __asm__("__loweringlib.internal.8");
void loweringlib_internal_ray_8() {}
void loweringlib_internal_ray_9() __asm__("__loweringlib.internal.9");
void loweringlib_internal_ray_9() {}
}
