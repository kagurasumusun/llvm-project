// metal_raytracing — MSL raytracing (cleanroom, comprehensive)
#ifndef _METAL_RAYTRACING_H_
#define _METAL_RAYTRACING_H_
#include <metal/metal_common>

namespace metal
{

// ============================================================================
// Forward declarations and opaque types
// ============================================================================
struct instance_acceleration_structure_t {};
struct primitive_acceleration_structure_t {};
struct intersection_function_table_t {};
struct visible_function_table_t {};
struct function_handle_t {};

enum class intersection_type : uint {
  triangle = 0,
  bounding_box = 1,
  curve = 2
};

// ============================================================================
// ray
// ============================================================================
struct ray {
  float3 origin;
  float3 direction;
  float min_distance;
  float max_distance;

  METAL_FUNC ray() thread = default;
  METAL_FUNC ray(float3 o, float3 d, float tmin, float tmax) thread
    : origin(o), direction(d), min_distance(tmin), max_distance(tmax) {}
};

// ============================================================================
// intersection_result — template specializations for each result type
// ============================================================================
template <uint N>
struct intersection_result {};

// Triangle intersection result (N=1: single level, N=2: multi-level instancing)
template <>
struct intersection_result<1> {
  METAL_FUNC uint get_instance_id() const thread { return __air_get_instance_id_intersection_result(); }
  METAL_FUNC uint get_user_instance_id() const thread { return __air_get_user_instance_id_intersection_result(); }
  METAL_FUNC uint get_instance_id(uint depth) const thread { return __air_get_instance_id_depth_intersection_result(depth); }
  METAL_FUNC uint get_user_instance_id(uint depth) const thread { return __air_get_user_instance_id_depth_intersection_result(depth); }
  METAL_FUNC float4x3 get_world_to_object_transform() const thread { return __air_get_world_to_object_transform_intersection_result(); }
  METAL_FUNC float4x3 get_object_to_world_transform() const thread { return __air_get_object_to_world_transform_intersection_result(); }
  METAL_FUNC intersection_type get_type() const thread { return (intersection_type)__air_get_type_intersection_result(); }
  METAL_FUNC float get_distance() const thread { return __air_get_distance_intersection_result(); }
  METAL_FUNC uint get_geometry_id() const thread { return __air_get_geometry_id_intersection_result(); }
  METAL_FUNC uint get_primitive_id() const thread { return __air_get_primitive_id_intersection_result(); }
  METAL_FUNC float3 get_ray_origin() const thread { return __air_get_ray_origin_intersection_result(); }
  METAL_FUNC float3 get_ray_direction() const thread { return __air_get_ray_direction_intersection_result(); }
  METAL_FUNC float get_ray_min_distance() const thread { return __air_get_ray_min_distance_intersection_result(); }
  METAL_FUNC bool is_triangle_front_facing() const thread { return __air_is_triangle_front_facing_intersection_result(); }
  METAL_FUNC float2 get_triangle_barycentric_coord() const thread { return __air_get_triangle_barycentric_coord_intersection_result(); }
  METAL_FUNC float get_curve_parameter() const thread { return __air_get_curve_parameter_intersection_result(); }
};

template <>
struct intersection_result<2> {
  METAL_FUNC uint get_instance_count() const thread { return __air_get_instance_count_intersection_result(); }
  METAL_FUNC uint get_instance_id() const thread { return __air_get_instance_id_intersection_result(); }
  METAL_FUNC uint get_user_instance_id() const thread { return __air_get_user_instance_id_intersection_result(); }
  METAL_FUNC uint get_instance_id(uint depth) const thread { return __air_get_instance_id_depth_intersection_result(depth); }
  METAL_FUNC uint get_user_instance_id(uint depth) const thread { return __air_get_user_instance_id_depth_intersection_result(depth); }
  METAL_FUNC float4x3 get_world_to_object_transform() const thread { return __air_get_world_to_object_transform_intersection_result(); }
  METAL_FUNC float4x3 get_object_to_world_transform() const thread { return __air_get_object_to_world_transform_intersection_result(); }
  METAL_FUNC intersection_type get_type() const thread { return (intersection_type)__air_get_type_intersection_result(); }
  METAL_FUNC float get_distance() const thread { return __air_get_distance_intersection_result(); }
  METAL_FUNC uint get_geometry_id() const thread { return __air_get_geometry_id_intersection_result(); }
  METAL_FUNC uint get_primitive_id() const thread { return __air_get_primitive_id_intersection_result(); }
  METAL_FUNC float3 get_ray_origin() const thread { return __air_get_ray_origin_intersection_result(); }
  METAL_FUNC float3 get_ray_direction() const thread { return __air_get_ray_direction_intersection_result(); }
  METAL_FUNC float get_ray_min_distance() const thread { return __air_get_ray_min_distance_intersection_result(); }
  METAL_FUNC bool is_triangle_front_facing() const thread { return __air_is_triangle_front_facing_intersection_result(); }
  METAL_FUNC float2 get_triangle_barycentric_coord() const thread { return __air_get_triangle_barycentric_coord_intersection_result(); }
  METAL_FUNC float get_curve_parameter() const thread { return __air_get_curve_parameter_intersection_result(); }
};

// ============================================================================
// intersection_params
// ============================================================================
struct intersection_params {
  METAL_FUNC constexpr intersection_params() thread = default;
  METAL_FUNC constexpr intersection_params(const intersection_params &) thread = default;
  METAL_FUNC constexpr intersection_params &operator=(const intersection_params &) thread = default;

  METAL_FUNC void set_geometry_intersection_function_table_index(uint idx) thread {
    __air_set_geometry_intersection_function_table_index_intersection_params(idx);
  }
  METAL_FUNC void set_force_opacity(forced_opacity_mode mode) thread {
    __air_set_force_opacity_intersection_params((uint)mode);
  }
  METAL_FUNC void set_intersection_type(intersection_type type) thread {
    __air_set_intersection_type_intersection_params((uint)type);
  }
};

enum class forced_opacity_mode : uint {
  force_opacity = 0,
  force_non_opacity = 1
};

// ============================================================================
// acceleration_structure (forward declaration — full definition later with AS ctors)
// ============================================================================
template <typename... Tags>
struct acceleration_structure;

using instance_acceleration_structure = acceleration_structure<instance_acceleration_structure_t>;
using primitive_acceleration_structure = acceleration_structure<primitive_acceleration_structure_t>;

// ============================================================================
// visibility_function_table (forward declaration — full definition later)
// ============================================================================
template <typename T = void>
struct visible_function_table;

// ============================================================================
// intersection_function_table (forward declaration — full definition later)
// ============================================================================
template <typename... Tags>
struct intersection_function_table;

// ============================================================================
// function_handle (forward declaration — full definition later with AS ctors)
// ============================================================================

// ============================================================================
// intersection_function_buffer_arguments
// ============================================================================
struct intersection_function_buffer_arguments {
  METAL_FUNC constexpr intersection_function_buffer_arguments() thread = default;
  METAL_FUNC constexpr intersection_function_buffer_arguments(const intersection_function_buffer_arguments &) thread = default;
};

// ============================================================================
// _intersector_base — base template for intersection queries
// ============================================================================
template <uint N, typename... Tags>
struct _intersector_base {
  METAL_FUNC constexpr _intersector_base() thread = default;
  METAL_FUNC constexpr _intersector_base(const _intersector_base &) thread = default;

  // reset variants
  METAL_FUNC void reset(ray r, primitive_acceleration_structure as) thread {
    __air_reset_intersection_query(r, as, 0xFF, intersection_params());
  }
  METAL_FUNC void reset(ray r, primitive_acceleration_structure as, intersection_params params) thread {
    __air_reset_intersection_query(r, as, 0xFF, params);
  }
  METAL_FUNC void reset(ray r, instance_acceleration_structure as) thread {
    __air_reset_intersection_query(r, as, 0xFF, intersection_params());
  }
  METAL_FUNC void reset(ray r, instance_acceleration_structure as, intersection_params params) thread {
    __air_reset_intersection_query(r, as, 0xFF, params);
  }
  METAL_FUNC void reset(ray r, instance_acceleration_structure as, uint mask) thread {
    __air_reset_intersection_query(r, as, mask, intersection_params());
  }
  METAL_FUNC void reset(ray r, instance_acceleration_structure as, uint mask, intersection_params params) thread {
    __air_reset_intersection_query(r, as, mask, params);
  }
  METAL_FUNC void reset(ray r, primitive_acceleration_structure as, uint mask, intersection_params params) thread {
    __air_reset_intersection_query(r, as, mask, params);
  }

  // Candidate queries
  METAL_FUNC uint get_candidate_instance_id() const thread { return __air_get_candidate_instance_id_intersection_query(); }
  METAL_FUNC uint get_candidate_user_instance_id() const thread { return __air_get_candidate_user_instance_id_intersection_query(); }
  METAL_FUNC float4x3 get_candidate_object_to_world_transform() const thread { return __air_get_candidate_object_to_world_transform_intersection_query(); }
  METAL_FUNC float4x3 get_candidate_world_to_object_transform() const thread { return __air_get_candidate_world_to_object_transform_intersection_query(); }
  METAL_FUNC uint get_candidate_instance_count() const thread { return __air_get_candidate_instance_count_intersection_query(); }
  METAL_FUNC uint get_candidate_instance_id(uint depth) const thread { return __air_get_candidate_instance_id_depth_intersection_query(depth); }
  METAL_FUNC uint get_candidate_user_instance_id(uint depth) const thread { return __air_get_candidate_user_instance_id_depth_intersection_query(depth); }
  METAL_FUNC float2 get_candidate_triangle_barycentric_coord() const thread { return __air_get_candidate_triangle_barycentric_coord_intersection_query(); }
  METAL_FUNC bool is_candidate_triangle_front_facing() const thread { return __air_is_candidate_triangle_front_facing_intersection_query(); }
  METAL_FUNC float get_candidate_curve_parameter() const thread { return __air_get_candidate_curve_parameter_intersection_query(); }
  METAL_FUNC intersection_type get_candidate_intersection_type() const thread { return (intersection_type)__air_get_candidate_intersection_type_intersection_query(); }
  METAL_FUNC float get_candidate_triangle_distance() const thread { return __air_get_candidate_triangle_distance_intersection_query(); }
  METAL_FUNC float get_candidate_curve_distance() const thread { return __air_get_candidate_curve_distance_intersection_query(); }
  METAL_FUNC bool is_candidate_non_opaque_bounding_box() const thread { return __air_is_candidate_non_opaque_bounding_box_intersection_query(); }
  METAL_FUNC uint get_candidate_geometry_id() const thread { return __air_get_candidate_geometry_id_intersection_query(); }
  METAL_FUNC uint get_candidate_primitive_id() const thread { return __air_get_candidate_primitive_id_intersection_query(); }
  METAL_FUNC float3 get_candidate_ray_origin() const thread { return __air_get_candidate_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_candidate_ray_direction() const thread { return __air_get_candidate_ray_direction_intersection_query(); }

  // Committed queries
  METAL_FUNC uint get_committed_instance_id() const thread { return __air_get_committed_instance_id_intersection_query(); }
  METAL_FUNC uint get_committed_user_instance_id() const thread { return __air_get_committed_user_instance_id_intersection_query(); }
  METAL_FUNC float4x3 get_committed_object_to_world_transform() const thread { return __air_get_committed_object_to_world_transform_intersection_query(); }
  METAL_FUNC float4x3 get_committed_world_to_object_transform() const thread { return __air_get_committed_world_to_object_transform_intersection_query(); }
  METAL_FUNC uint get_committed_instance_count() const thread { return __air_get_committed_instance_count_intersection_query(); }
  METAL_FUNC uint get_committed_instance_id(uint depth) const thread { return __air_get_committed_instance_id_depth_intersection_query(depth); }
  METAL_FUNC uint get_committed_user_instance_id(uint depth) const thread { return __air_get_committed_user_instance_id_depth_intersection_query(depth); }
  METAL_FUNC float2 get_committed_triangle_barycentric_coord() const thread { return __air_get_committed_triangle_barycentric_coord_intersection_query(); }
  METAL_FUNC bool is_committed_triangle_front_facing() const thread { return __air_is_committed_triangle_front_facing_intersection_query(); }
  METAL_FUNC float get_committed_curve_parameter() const thread { return __air_get_committed_curve_parameter_intersection_query(); }
  METAL_FUNC intersection_type get_committed_intersection_type() const thread { return (intersection_type)__air_get_committed_intersection_type_intersection_query(); }
  METAL_FUNC float get_committed_distance() const thread { return __air_get_committed_distance_intersection_query(); }
  METAL_FUNC uint get_committed_geometry_id() const thread { return __air_get_committed_geometry_id_intersection_query(); }
  METAL_FUNC uint get_committed_primitive_id() const thread { return __air_get_committed_primitive_id_intersection_query(); }
  METAL_FUNC float3 get_committed_ray_origin() const thread { return __air_get_committed_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_committed_ray_direction() const thread { return __air_get_committed_ray_direction_intersection_query(); }

  // Query control
  METAL_FUNC bool next() thread { return __air_next_intersection_query(); }
  METAL_FUNC void abort() thread { __air_abort_intersection_query(); }
  METAL_FUNC void commit_triangle_intersection() thread { __air_commit_triangle_intersection_intersection_query(); }
  METAL_FUNC void commit_curve_intersection() thread { __air_commit_curve_intersection_intersection_query(); }
  METAL_FUNC void commit_bounding_box_intersection(float distance) thread {
    __air_commit_bounding_box_intersection_intersection_query(distance);
  }

  // Result extraction
  METAL_FUNC intersection_result<N> get() const thread {
    return intersection_result<N>();
  }
  METAL_FUNC intersection_params get_intersection_params() const thread { return intersection_params(); }
  METAL_FUNC float3 get_world_space_ray_origin() const thread { return __air_get_world_space_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_world_space_ray_direction() const thread { return __air_get_world_space_ray_direction_intersection_query(); }
};

// ============================================================================
// intersector — main user-facing template
// ============================================================================
template <uint N = 1, bool ForceBoundingBox = false, typename... Tags>
struct intersector : _intersector_base<N, Tags...> {
  using result_type = intersection_result<N>;

  METAL_FUNC constexpr intersector() thread = default;
  METAL_FUNC constexpr intersector(const intersector &) thread = default;

  // Primitive-only intersect
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }

  // Instance intersect (with mask and time)
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask) thread {
    this->reset(r, as, mask);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as, mask);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as, mask);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as, mask);
    this->next();
    return this->get();
  }

  // Acceleration structure (generic, with time)
  METAL_FUNC result_type intersect(ray r, acceleration_structure<> as, float time) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, acceleration_structure<> as, float time,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, acceleration_structure<> as, float time,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, acceleration_structure<> as, float time,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, acceleration_structure<> as, uint mask, float time) thread {
    this->reset(r, as, mask);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, acceleration_structure<> as, uint mask, float time,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as, mask);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, acceleration_structure<> as, uint mask, float time,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as, mask);
    this->next();
    return this->get();
  }
  METAL_FUNC result_type intersect(ray r, acceleration_structure<> as, uint mask, float time,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as, mask);
    this->next();
    return this->get();
  }
};

// ============================================================================
// intersection_query (legacy API, template-based)
// ============================================================================
template <intersection_type Type, uint N = 1>
struct intersection_query {
  METAL_FUNC void reset() thread { __air_reset_intersection_query_legacy(); }
  METAL_FUNC bool next() thread { return __air_next_intersection_query_legacy(); }
  METAL_FUNC void abort() thread { __air_abort_intersection_query_legacy(); }

  METAL_FUNC uint get_candidate_instance_id() const thread { return __air_get_candidate_instance_id_intersection_query(); }
  METAL_FUNC uint get_candidate_user_instance_id() const thread { return __air_get_candidate_user_instance_id_intersection_query(); }
  METAL_FUNC float4x3 get_candidate_object_to_world_transform() const thread { return __air_get_candidate_object_to_world_transform_intersection_query(); }
  METAL_FUNC float4x3 get_candidate_world_to_object_transform() const thread { return __air_get_candidate_world_to_object_transform_intersection_query(); }
  METAL_FUNC uint get_committed_instance_id() const thread { return __air_get_committed_instance_id_intersection_query(); }
  METAL_FUNC uint get_committed_user_instance_id() const thread { return __air_get_committed_user_instance_id_intersection_query(); }
  METAL_FUNC float4x3 get_committed_object_to_world_transform() const thread { return __air_get_committed_object_to_world_transform_intersection_query(); }
  METAL_FUNC float4x3 get_committed_world_to_object_transform() const thread { return __air_get_committed_world_to_object_transform_intersection_query(); }
  METAL_FUNC uint get_candidate_instance_count() const thread { return __air_get_candidate_instance_count_intersection_query(); }
  METAL_FUNC uint get_candidate_instance_id(uint depth) const thread { return __air_get_candidate_instance_id_depth_intersection_query(depth); }
  METAL_FUNC uint get_candidate_user_instance_id(uint depth) const thread { return __air_get_candidate_user_instance_id_depth_intersection_query(depth); }
  METAL_FUNC uint get_committed_instance_count() const thread { return __air_get_committed_instance_count_intersection_query(); }
  METAL_FUNC uint get_committed_instance_id(uint depth) const thread { return __air_get_committed_instance_id_depth_intersection_query(depth); }
  METAL_FUNC uint get_committed_user_instance_id(uint depth) const thread { return __air_get_committed_user_instance_id_depth_intersection_query(depth); }
  METAL_FUNC float2 get_candidate_triangle_barycentric_coord() const thread { return __air_get_candidate_triangle_barycentric_coord_intersection_query(); }
  METAL_FUNC bool is_candidate_triangle_front_facing() const thread { return __air_is_candidate_triangle_front_facing_intersection_query(); }
  METAL_FUNC float2 get_committed_triangle_barycentric_coord() const thread { return __air_get_committed_triangle_barycentric_coord_intersection_query(); }
  METAL_FUNC bool is_committed_triangle_front_facing() const thread { return __air_is_committed_triangle_front_facing_intersection_query(); }
  METAL_FUNC float get_candidate_curve_parameter() const thread { return __air_get_candidate_curve_parameter_intersection_query(); }
  METAL_FUNC float get_committed_curve_parameter() const thread { return __air_get_committed_curve_parameter_intersection_query(); }
  METAL_FUNC intersection_type get_candidate_intersection_type() const thread { return (intersection_type)__air_get_candidate_intersection_type_intersection_query(); }
  METAL_FUNC float get_candidate_triangle_distance() const thread { return __air_get_candidate_triangle_distance_intersection_query(); }
  METAL_FUNC float get_candidate_curve_distance() const thread { return __air_get_candidate_curve_distance_intersection_query(); }
  METAL_FUNC bool is_candidate_non_opaque_bounding_box() const thread { return __air_is_candidate_non_opaque_bounding_box_intersection_query(); }
  METAL_FUNC uint get_candidate_geometry_id() const thread { return __air_get_candidate_geometry_id_intersection_query(); }
  METAL_FUNC uint get_candidate_primitive_id() const thread { return __air_get_candidate_primitive_id_intersection_query(); }
  METAL_FUNC float3 get_candidate_ray_origin() const thread { return __air_get_candidate_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_candidate_ray_direction() const thread { return __air_get_candidate_ray_direction_intersection_query(); }
  METAL_FUNC intersection_type get_committed_intersection_type() const thread { return (intersection_type)__air_get_committed_intersection_type_intersection_query(); }
  METAL_FUNC float get_committed_distance() const thread { return __air_get_committed_distance_intersection_query(); }
  METAL_FUNC uint get_committed_geometry_id() const thread { return __air_get_committed_geometry_id_intersection_query(); }
  METAL_FUNC uint get_committed_primitive_id() const thread { return __air_get_committed_primitive_id_intersection_query(); }
  METAL_FUNC float3 get_committed_ray_origin() const thread { return __air_get_committed_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_committed_ray_direction() const thread { return __air_get_committed_ray_direction_intersection_query(); }

  METAL_FUNC void commit_triangle_intersection() thread { __air_commit_triangle_intersection_intersection_query(); }
  METAL_FUNC void commit_curve_intersection() thread { __air_commit_curve_intersection_intersection_query(); }
  METAL_FUNC void commit_bounding_box_intersection(float distance) thread {
    __air_commit_bounding_box_intersection_intersection_query(distance);
  }
  METAL_FUNC intersection_params get_intersection_params() const thread { return intersection_params(); }
  METAL_FUNC float3 get_world_space_ray_origin() const thread { return __air_get_world_space_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_world_space_ray_direction() const thread { return __air_get_world_space_ray_direction_intersection_query(); }

  METAL_FUNC intersection_result<N> get() const thread { return intersection_result<N>(); }
};

// ============================================================================
// Null handle queries
// ============================================================================
template <typename... Tags>
METAL_FUNC bool is_null_acceleration_structure(acceleration_structure<Tags...> as) { return __air_is_null_instance_acceleration_structure(); }
METAL_FUNC bool is_null_instance_acceleration_structure(instance_acceleration_structure as) { return __air_is_null_instance_acceleration_structure(); }
METAL_FUNC bool is_null_primitive_acceleration_structure(primitive_acceleration_structure as) { return __air_is_null_primitive_acceleration_structure(); }
template <typename... Tags>
METAL_FUNC bool is_null_intersection_function_table(intersection_function_table<Tags...> t) { return __air_is_null_intersection_function_table(); }

// ============================================================================
// Template helpers for typed intersection
// ============================================================================
template <typename... Tags>
METAL_FUNC bool operator==(acceleration_structure<Tags...>, acceleration_structure<Tags...>) { return true; }
template <typename... Tags>
METAL_FUNC bool operator!=(acceleration_structure<Tags...>, acceleration_structure<Tags...>) { return false; }

// ============================================================================
// intersection_result helper for intersector
// ============================================================================
template <>
struct intersection_result<0> {};

// ============================================================================
// Additional intersection_result specializations (3+: curve data)
// ============================================================================
template <>
struct intersection_result<3> {
  METAL_FUNC uint get_instance_id() const thread { return __air_get_instance_id_intersection_result(); }
  METAL_FUNC uint get_user_instance_id() const thread { return __air_get_user_instance_id_intersection_result(); }
  METAL_FUNC uint get_instance_id(uint depth) const thread { return __air_get_instance_id_depth_intersection_result(depth); }
  METAL_FUNC uint get_user_instance_id(uint depth) const thread { return __air_get_user_instance_id_depth_intersection_result(depth); }
  METAL_FUNC float4x3 get_world_to_object_transform() const thread { return __air_get_world_to_object_transform_intersection_result(); }
  METAL_FUNC float4x3 get_object_to_world_transform() const thread { return __air_get_object_to_world_transform_intersection_result(); }
  METAL_FUNC intersection_type get_type() const thread { return (intersection_type)__air_get_type_intersection_result(); }
  METAL_FUNC float get_distance() const thread { return __air_get_distance_intersection_result(); }
  METAL_FUNC uint get_geometry_id() const thread { return __air_get_geometry_id_intersection_result(); }
  METAL_FUNC uint get_primitive_id() const thread { return __air_get_primitive_id_intersection_result(); }
  METAL_FUNC float3 get_ray_origin() const thread { return __air_get_ray_origin_intersection_result(); }
  METAL_FUNC float3 get_ray_direction() const thread { return __air_get_ray_direction_intersection_result(); }
  METAL_FUNC float get_ray_min_distance() const thread { return __air_get_ray_min_distance_intersection_result(); }
  METAL_FUNC float get_curve_parameter() const thread { return __air_get_curve_parameter_intersection_result(); }
  METAL_FUNC float get_curve_bounding_box_end_distance() const thread { return __air_get_curve_bounding_box_end_distance_intersection_result(); }
};

template <>
struct intersection_result<4> {
  METAL_FUNC uint get_instance_count() const thread { return __air_get_instance_count_intersection_result(); }
  METAL_FUNC uint get_instance_id() const thread { return __air_get_instance_id_intersection_result(); }
  METAL_FUNC uint get_user_instance_id() const thread { return __air_get_user_instance_id_intersection_result(); }
  METAL_FUNC uint get_instance_id(uint depth) const thread { return __air_get_instance_id_depth_intersection_result(depth); }
  METAL_FUNC uint get_user_instance_id(uint depth) const thread { return __air_get_user_instance_id_depth_intersection_result(depth); }
  METAL_FUNC float4x3 get_world_to_object_transform() const thread { return __air_get_world_to_object_transform_intersection_result(); }
  METAL_FUNC float4x3 get_object_to_world_transform() const thread { return __air_get_object_to_world_transform_intersection_result(); }
  METAL_FUNC intersection_type get_type() const thread { return (intersection_type)__air_get_type_intersection_result(); }
  METAL_FUNC float get_distance() const thread { return __air_get_distance_intersection_result(); }
  METAL_FUNC uint get_geometry_id() const thread { return __air_get_geometry_id_intersection_result(); }
  METAL_FUNC uint get_primitive_id() const thread { return __air_get_primitive_id_intersection_result(); }
  METAL_FUNC float3 get_ray_origin() const thread { return __air_get_ray_origin_intersection_result(); }
  METAL_FUNC float3 get_ray_direction() const thread { return __air_get_ray_direction_intersection_result(); }
  METAL_FUNC float get_ray_min_distance() const thread { return __air_get_ray_min_distance_intersection_result(); }
  METAL_FUNC float get_curve_parameter() const thread { return __air_get_curve_parameter_intersection_result(); }
  METAL_FUNC float get_curve_bounding_box_end_distance() const thread { return __air_get_curve_bounding_box_end_distance_intersection_result(); }
};

// ============================================================================
// Re-export common intersection_result specializations
// ============================================================================
using intersection_result_triangle = intersection_result<1>;
using intersection_result_triangle_instancing = intersection_result<2>;
using intersection_result_curve = intersection_result<3>;
using intersection_result_curve_instancing = intersection_result<4>;

// ============================================================================
// _intersector_base_ift — with intersection function table
// ============================================================================
template <uint N, typename... Tags>
struct _intersector_base_ift : _intersector_base<N, Tags...> {
  METAL_FUNC constexpr _intersector_base_ift() thread = default;
  METAL_FUNC constexpr _intersector_base_ift(const _intersector_base_ift &) thread = default;

  // intersect with function table
  template <typename AS, typename F>
  METAL_FUNC void intersect(ray r, AS as, uint mask, float time,
                             __air_intersection_function_table_t ft, F callback) thread {
    __air_intersect_with_ift(r, as, mask, time, ft, callback);
  }
  template <typename AS, typename F, typename P>
  METAL_FUNC void intersect(ray r, AS as, uint mask, float time,
                             __air_intersection_function_table_t ft, F callback, const P *payload) thread {
    __air_intersect_with_ift_payload(r, as, mask, time, ft, callback, payload);
  }
};

// ============================================================================
// _intersector_base_ifb — with intersection function buffer
// ============================================================================
template <uint N, typename... Tags>
struct _intersector_base_ifb : _intersector_base<N, Tags...> {
  METAL_FUNC constexpr _intersector_base_ifb() thread = default;
  METAL_FUNC constexpr _intersector_base_ifb(const _intersector_base_ifb &) thread = default;

  // intersect with function buffer
  template <typename AS, typename F>
  METAL_FUNC void intersect(ray r, AS as, uint mask, float time,
                             intersection_function_buffer_arguments ifba,
                             const void *user_data_buffer, F callback) thread {
    __air_intersect_with_ifb(r, as, mask, time, ifba, user_data_buffer, callback);
  }
  template <typename AS, typename F, typename P>
  METAL_FUNC void intersect(ray r, AS as, uint mask, float time,
                             intersection_function_buffer_arguments ifba,
                             const void *user_data_buffer, F callback, const P *payload) thread {
    __air_intersect_with_ifb_payload(r, as, mask, time, ifba, user_data_buffer, callback, payload);
  }
};

// ============================================================================
// Additional _intersector_base specializations
// ============================================================================
#define _METAL_INTERSECTOR_BASE_EXTRA(N_VAL, NAME) \
  template <typename... Tags> \
  struct _intersector_base_##NAME : _intersector_base<N_VAL, Tags...> { \
    METAL_FUNC constexpr _intersector_base_##NAME() thread = default; \
    METAL_FUNC constexpr _intersector_base_##NAME(const _intersector_base_##NAME &) thread = default; \
  };

_METAL_INTERSECTOR_BASE_EXTRA(1, inst_triangle)
_METAL_INTERSECTOR_BASE_EXTRA(2, mli_triangle)
_METAL_INTERSECTOR_BASE_EXTRA(3, inst_curve)
_METAL_INTERSECTOR_BASE_EXTRA(4, mli_curve)

#undef _METAL_INTERSECTOR_BASE_EXTRA

// ============================================================================
// intersector_ift — user-facing intersector with function table support
// ============================================================================
template <uint N = 1, bool ForceBoundingBox = false, typename... Tags>
struct intersector_ift : _intersector_base_ift<N, Tags...> {
  using result_type = intersection_result<N>;

  METAL_FUNC constexpr intersector_ift() thread = default;
  METAL_FUNC constexpr intersector_ift(const intersector_ift &) thread = default;

  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask) thread {
    this->reset(r, as, mask); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as, mask); this->next(); return this->get();
  }
};

// ============================================================================
// intersector_ifb — user-facing intersector with function buffer support
// ============================================================================
template <uint N = 1, bool ForceBoundingBox = false, typename... Tags>
struct intersector_ifb : _intersector_base_ifb<N, Tags...> {
  using result_type = intersection_result<N>;

  METAL_FUNC constexpr intersector_ifb() thread = default;
  METAL_FUNC constexpr intersector_ifb(const intersector_ifb &) thread = default;

  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as, mask); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as, mask); this->next(); return this->get();
  }
};

// ============================================================================
// Address-space-qualified acceleration_structure
// ============================================================================
template <typename... Tags>
struct acceleration_structure {
  METAL_FUNC acceleration_structure() thread = default;
  METAL_FUNC acceleration_structure() device = default;
  METAL_FUNC acceleration_structure() constant = default;
  METAL_FUNC acceleration_structure() ray_data = default;
  METAL_FUNC acceleration_structure() object_data = default;

  METAL_FUNC acceleration_structure(const thread acceleration_structure &) thread = default;
  METAL_FUNC acceleration_structure(const device acceleration_structure &) thread = default;
  METAL_FUNC acceleration_structure(const device coherent(device) acceleration_structure &) thread = default;
  METAL_FUNC acceleration_structure(const constant acceleration_structure &) thread = default;
  METAL_FUNC acceleration_structure(const ray_data acceleration_structure &) thread = default;
  METAL_FUNC acceleration_structure(const object_data acceleration_structure &) thread = default;

  METAL_FUNC acceleration_structure(const thread acceleration_structure &) constant = default;
  METAL_FUNC acceleration_structure(const device acceleration_structure &) constant = default;
  METAL_FUNC acceleration_structure(const device coherent(device) acceleration_structure &) constant = default;
  METAL_FUNC acceleration_structure(const constant acceleration_structure &) constant = default;
  METAL_FUNC acceleration_structure(const ray_data acceleration_structure &) constant = default;
  METAL_FUNC acceleration_structure(const object_data acceleration_structure &) constant = default;

  METAL_FUNC acceleration_structure(const thread acceleration_structure &) ray_data = default;
  METAL_FUNC acceleration_structure(const device acceleration_structure &) ray_data = default;
  METAL_FUNC acceleration_structure(const device coherent(device) acceleration_structure &) ray_data = default;
  METAL_FUNC acceleration_structure(const constant acceleration_structure &) ray_data = default;
  METAL_FUNC acceleration_structure(const ray_data acceleration_structure &) ray_data = default;
  METAL_FUNC acceleration_structure(const object_data acceleration_structure &) ray_data = default;

  METAL_FUNC acceleration_structure(const thread acceleration_structure &) device = default;
  METAL_FUNC acceleration_structure(const device acceleration_structure &) device = default;
  METAL_FUNC acceleration_structure(const device coherent(device) acceleration_structure &) device = default;
  METAL_FUNC acceleration_structure(const constant acceleration_structure &) device = default;
  METAL_FUNC acceleration_structure(const ray_data acceleration_structure &) device = default;
  METAL_FUNC acceleration_structure(const object_data acceleration_structure &) device = default;

  METAL_FUNC acceleration_structure(const thread acceleration_structure &) device coherent(device) = default;
  METAL_FUNC acceleration_structure(const device acceleration_structure &) device coherent(device) = default;
  METAL_FUNC acceleration_structure(const device coherent(device) acceleration_structure &) device coherent(device) = default;
  METAL_FUNC acceleration_structure(const constant acceleration_structure &) device coherent(device) = default;
  METAL_FUNC acceleration_structure(const ray_data acceleration_structure &) device coherent(device) = default;
  METAL_FUNC acceleration_structure(const object_data acceleration_structure &) device coherent(device) = default;

  METAL_FUNC acceleration_structure(const thread acceleration_structure &) object_data = default;
  METAL_FUNC acceleration_structure(const device acceleration_structure &) object_data = default;
  METAL_FUNC acceleration_structure(const device coherent(device) acceleration_structure &) object_data = default;
  METAL_FUNC acceleration_structure(const constant acceleration_structure &) object_data = default;
  METAL_FUNC acceleration_structure(const ray_data acceleration_structure &) object_data = default;
  METAL_FUNC acceleration_structure(const object_data acceleration_structure &) object_data = default;

  // Assignment operators for all AS pairs
  METAL_FUNC thread acceleration_structure &operator=(const thread acceleration_structure &) thread = default;
  METAL_FUNC thread acceleration_structure &operator=(const device acceleration_structure &) thread = default;
  METAL_FUNC thread acceleration_structure &operator=(const device coherent(device) acceleration_structure &) thread = default;
  METAL_FUNC thread acceleration_structure &operator=(const constant acceleration_structure &) thread = default;
  METAL_FUNC thread acceleration_structure &operator=(const ray_data acceleration_structure &) thread = default;
  METAL_FUNC thread acceleration_structure &operator=(const object_data acceleration_structure &) thread = default;
  METAL_FUNC device acceleration_structure &operator=(const thread acceleration_structure &) device = default;
  METAL_FUNC device acceleration_structure &operator=(const device acceleration_structure &) device = default;
  METAL_FUNC device acceleration_structure &operator=(const device coherent(device) acceleration_structure &) device = default;
  METAL_FUNC device acceleration_structure &operator=(const constant acceleration_structure &) device = default;
  METAL_FUNC device acceleration_structure &operator=(const ray_data acceleration_structure &) device = default;
  METAL_FUNC device acceleration_structure &operator=(const object_data acceleration_structure &) device = default;
  METAL_FUNC device coherent(device) acceleration_structure &operator=(const thread acceleration_structure &) device coherent(device) = default;
  METAL_FUNC device coherent(device) acceleration_structure &operator=(const device acceleration_structure &) device coherent(device) = default;
  METAL_FUNC device coherent(device) acceleration_structure &operator=(const device coherent(device) acceleration_structure &) device coherent(device) = default;
  METAL_FUNC device coherent(device) acceleration_structure &operator=(const constant acceleration_structure &) device coherent(device) = default;
  METAL_FUNC device coherent(device) acceleration_structure &operator=(const ray_data acceleration_structure &) device coherent(device) = default;
  METAL_FUNC device coherent(device) acceleration_structure &operator=(const object_data acceleration_structure &) device coherent(device) = default;
  METAL_FUNC constant acceleration_structure &operator=(const thread acceleration_structure &) constant = default;
  METAL_FUNC constant acceleration_structure &operator=(const device acceleration_structure &) constant = default;
  METAL_FUNC constant acceleration_structure &operator=(const device coherent(device) acceleration_structure &) constant = default;
  METAL_FUNC constant acceleration_structure &operator=(const constant acceleration_structure &) constant = default;
  METAL_FUNC constant acceleration_structure &operator=(const ray_data acceleration_structure &) constant = default;
  METAL_FUNC constant acceleration_structure &operator=(const object_data acceleration_structure &) constant = default;
  METAL_FUNC ray_data acceleration_structure &operator=(const thread acceleration_structure &) ray_data = default;
  METAL_FUNC ray_data acceleration_structure &operator=(const device acceleration_structure &) ray_data = default;
  METAL_FUNC ray_data acceleration_structure &operator=(const device coherent(device) acceleration_structure &) ray_data = default;
  METAL_FUNC ray_data acceleration_structure &operator=(const constant acceleration_structure &) ray_data = default;
  METAL_FUNC ray_data acceleration_structure &operator=(const ray_data acceleration_structure &) ray_data = default;
  METAL_FUNC ray_data acceleration_structure &operator=(const object_data acceleration_structure &) ray_data = default;
  METAL_FUNC object_data acceleration_structure &operator=(const thread acceleration_structure &) object_data = default;
  METAL_FUNC object_data acceleration_structure &operator=(const device acceleration_structure &) object_data = default;
  METAL_FUNC object_data acceleration_structure &operator=(const device coherent(device) acceleration_structure &) object_data = default;
  METAL_FUNC object_data acceleration_structure &operator=(const constant acceleration_structure &) object_data = default;
  METAL_FUNC object_data acceleration_structure &operator=(const ray_data acceleration_structure &) object_data = default;
  METAL_FUNC object_data acceleration_structure &operator=(const object_data acceleration_structure &) object_data = default;

  // Handle queries
  METAL_FUNC bool is_null_instance() const thread { return __air_is_null_instance_acceleration_structure(); }
  METAL_FUNC bool is_null_primitive() const thread { return __air_is_null_primitive_acceleration_structure(); }
  METAL_FUNC uint get_resource_id_instance() const thread { return __air_get_resource_id_instance_acceleration_structure(); }
  METAL_FUNC uint get_resource_id_primitive() const thread { return __air_get_resource_id_primitive_acceleration_structure(); }
};

// ============================================================================
// Address-space-qualified visible_function_table
// ============================================================================
template <typename T>
struct visible_function_table {
  METAL_FUNC visible_function_table() thread = default;
  METAL_FUNC visible_function_table() device = default;
  METAL_FUNC visible_function_table() constant = default;
  METAL_FUNC visible_function_table() ray_data = default;
  METAL_FUNC visible_function_table() object_data = default;

  METAL_FUNC visible_function_table(const thread visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const device visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const device coherent(device) visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const constant visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const ray_data visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const object_data visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const thread visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const device visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const device coherent(device) visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const constant visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const ray_data visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const object_data visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const thread visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const device visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const device coherent(device) visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const constant visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const ray_data visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const object_data visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const thread visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const device visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const device coherent(device) visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const constant visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const ray_data visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const object_data visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const thread visible_function_table &) object_data = default;
  METAL_FUNC visible_function_table(const device visible_function_table &) object_data = default;
  METAL_FUNC visible_function_table(const device coherent(device) visible_function_table &) object_data = default;
  METAL_FUNC visible_function_table(const constant visible_function_table &) object_data = default;
  METAL_FUNC visible_function_table(const ray_data visible_function_table &) object_data = default;
  METAL_FUNC visible_function_table(const object_data visible_function_table &) object_data = default;

  // Assignment operators
  METAL_FUNC thread visible_function_table &operator=(const thread visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const device visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const device coherent(device) visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const constant visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const ray_data visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const object_data visible_function_table &) thread = default;
  METAL_FUNC device visible_function_table &operator=(const thread visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const device visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const device coherent(device) visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const constant visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const ray_data visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const object_data visible_function_table &) device = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const thread visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const device visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const device coherent(device) visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const constant visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const ray_data visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const object_data visible_function_table &) device coherent(device) = default;
  METAL_FUNC constant visible_function_table &operator=(const thread visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const device visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const device coherent(device) visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const constant visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const ray_data visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const object_data visible_function_table &) constant = default;
  METAL_FUNC ray_data visible_function_table &operator=(const thread visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const device visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const device coherent(device) visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const constant visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const ray_data visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const object_data visible_function_table &) ray_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const thread visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const device visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const device coherent(device) visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const constant visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const ray_data visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const object_data visible_function_table &) object_data = default;

  METAL_FUNC bool empty() const thread { return __air_is_null_visible_function_table(t); }
  METAL_FUNC uint size() const thread { return __air_get_size_visible_function_table(t); }
  METAL_FUNC function_handle get_function_handle(uint index) const thread {
    return __air_get_function_handle_visible_function_table(t, index);
  }
private:
  __air_visible_function_table_t t;
};

} // namespace metal
#endif // _METAL_RAYTRACING_H_

// ============================================================================
// Additional intersector variants with different tag combinations
// ============================================================================

// intersector with instancing for triangle data
template <typename... Tags>
struct intersector<1, false, Tags...> : _intersector_base<1, Tags...> {
  using result_type = intersection_result<1>;
  METAL_FUNC constexpr intersector() thread = default;
  METAL_FUNC constexpr intersector(const intersector &) thread = default;

  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask) thread {
    this->reset(r, as, mask); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as, mask); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as, mask); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask,
                                     intersection_function_buffer_arguments ifba,
                                     const void *user_data_buffer) thread {
    this->reset(r, as, mask); this->next(); return this->get();
  }
};

// intersector with bounding box for curve data
template <typename... Tags>
struct intersector<1, true, Tags...> : _intersector_base<1, Tags...> {
  using result_type = intersection_result<1>;
  METAL_FUNC constexpr intersector() thread = default;
  METAL_FUNC constexpr intersector(const intersector &) thread = default;

  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_table<> ft) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, primitive_acceleration_structure as,
                                     intersection_function_buffer_arguments ifba) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as) thread {
    this->reset(r, as); this->next(); return this->get();
  }
  METAL_FUNC result_type intersect(ray r, instance_acceleration_structure as, uint mask) thread {
    this->reset(r, as, mask); this->next(); return this->get();
  }
};

// ============================================================================
// Additional intersection_query specializations for multi-level instancing
// ============================================================================
template <intersection_type Type>
struct intersection_query<Type, 2> {
  METAL_FUNC void reset() thread { __air_reset_intersection_query_legacy(); }
  METAL_FUNC bool next() thread { return __air_next_intersection_query_legacy(); }
  METAL_FUNC void abort() thread { __air_abort_intersection_query_legacy(); }

  METAL_FUNC uint get_instance_count() const thread { return __air_get_instance_count_intersection_query(); }
  METAL_FUNC uint get_instance_id() const thread { return __air_get_instance_id_intersection_query(); }
  METAL_FUNC uint get_user_instance_id() const thread { return __air_get_user_instance_id_intersection_query(); }
  METAL_FUNC uint get_instance_id(uint depth) const thread { return __air_get_instance_id_depth_intersection_query(depth); }
  METAL_FUNC uint get_user_instance_id(uint depth) const thread { return __air_get_user_instance_id_depth_intersection_query(depth); }
  METAL_FUNC float4x3 get_world_to_object_transform() const thread { return __air_get_world_to_object_transform_intersection_query(); }
  METAL_FUNC float4x3 get_object_to_world_transform() const thread { return __air_get_object_to_world_transform_intersection_query(); }
  METAL_FUNC intersection_type get_type() const thread { return (intersection_type)__air_get_type_intersection_query(); }
  METAL_FUNC float get_distance() const thread { return __air_get_distance_intersection_query(); }
  METAL_FUNC uint get_geometry_id() const thread { return __air_get_geometry_id_intersection_query(); }
  METAL_FUNC uint get_primitive_id() const thread { return __air_get_primitive_id_intersection_query(); }
  METAL_FUNC float3 get_ray_origin() const thread { return __air_get_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_ray_direction() const thread { return __air_get_ray_direction_intersection_query(); }
  METAL_FUNC float get_ray_min_distance() const thread { return __air_get_ray_min_distance_intersection_query(); }
  METAL_FUNC bool is_triangle_front_facing() const thread { return __air_is_triangle_front_facing_intersection_query(); }
  METAL_FUNC float2 get_triangle_barycentric_coord() const thread { return __air_get_triangle_barycentric_coord_intersection_query(); }
  METAL_FUNC float get_curve_parameter() const thread { return __air_get_curve_parameter_intersection_query(); }

  // Candidate queries
  METAL_FUNC uint get_candidate_instance_id() const thread { return __air_get_candidate_instance_id_intersection_query(); }
  METAL_FUNC uint get_candidate_user_instance_id() const thread { return __air_get_candidate_user_instance_id_intersection_query(); }
  METAL_FUNC float4x3 get_candidate_object_to_world_transform() const thread { return __air_get_candidate_object_to_world_transform_intersection_query(); }
  METAL_FUNC float4x3 get_candidate_world_to_object_transform() const thread { return __air_get_candidate_world_to_object_transform_intersection_query(); }
  METAL_FUNC uint get_candidate_instance_count() const thread { return __air_get_candidate_instance_count_intersection_query(); }
  METAL_FUNC uint get_candidate_instance_id(uint depth) const thread { return __air_get_candidate_instance_id_depth_intersection_query(depth); }
  METAL_FUNC uint get_candidate_user_instance_id(uint depth) const thread { return __air_get_candidate_user_instance_id_depth_intersection_query(depth); }
  METAL_FUNC float2 get_candidate_triangle_barycentric_coord() const thread { return __air_get_candidate_triangle_barycentric_coord_intersection_query(); }
  METAL_FUNC bool is_candidate_triangle_front_facing() const thread { return __air_is_candidate_triangle_front_facing_intersection_query(); }
  METAL_FUNC float get_candidate_curve_parameter() const thread { return __air_get_candidate_curve_parameter_intersection_query(); }
  METAL_FUNC intersection_type get_candidate_intersection_type() const thread { return (intersection_type)__air_get_candidate_intersection_type_intersection_query(); }
  METAL_FUNC float get_candidate_triangle_distance() const thread { return __air_get_candidate_triangle_distance_intersection_query(); }
  METAL_FUNC float get_candidate_curve_distance() const thread { return __air_get_candidate_curve_distance_intersection_query(); }
  METAL_FUNC bool is_candidate_non_opaque_bounding_box() const thread { return __air_is_candidate_non_opaque_bounding_box_intersection_query(); }
  METAL_FUNC uint get_candidate_geometry_id() const thread { return __air_get_candidate_geometry_id_intersection_query(); }
  METAL_FUNC uint get_candidate_primitive_id() const thread { return __air_get_candidate_primitive_id_intersection_query(); }
  METAL_FUNC float3 get_candidate_ray_origin() const thread { return __air_get_candidate_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_candidate_ray_direction() const thread { return __air_get_candidate_ray_direction_intersection_query(); }

  // Committed queries
  METAL_FUNC uint get_committed_instance_id() const thread { return __air_get_committed_instance_id_intersection_query(); }
  METAL_FUNC uint get_committed_user_instance_id() const thread { return __air_get_committed_user_instance_id_intersection_query(); }
  METAL_FUNC float4x3 get_committed_object_to_world_transform() const thread { return __air_get_committed_object_to_world_transform_intersection_query(); }
  METAL_FUNC float4x3 get_committed_world_to_object_transform() const thread { return __air_get_committed_world_to_object_transform_intersection_query(); }
  METAL_FUNC uint get_committed_instance_count() const thread { return __air_get_committed_instance_count_intersection_query(); }
  METAL_FUNC uint get_committed_instance_id(uint depth) const thread { return __air_get_committed_instance_id_depth_intersection_query(depth); }
  METAL_FUNC uint get_committed_user_instance_id(uint depth) const thread { return __air_get_committed_user_instance_id_depth_intersection_query(depth); }
  METAL_FUNC float2 get_committed_triangle_barycentric_coord() const thread { return __air_get_committed_triangle_barycentric_coord_intersection_query(); }
  METAL_FUNC bool is_committed_triangle_front_facing() const thread { return __air_is_committed_triangle_front_facing_intersection_query(); }
  METAL_FUNC float get_committed_curve_parameter() const thread { return __air_get_committed_curve_parameter_intersection_query(); }
  METAL_FUNC intersection_type get_committed_intersection_type() const thread { return (intersection_type)__air_get_committed_intersection_type_intersection_query(); }
  METAL_FUNC float get_committed_distance() const thread { return __air_get_committed_distance_intersection_query(); }
  METAL_FUNC uint get_committed_geometry_id() const thread { return __air_get_committed_geometry_id_intersection_query(); }
  METAL_FUNC uint get_committed_primitive_id() const thread { return __air_get_committed_primitive_id_intersection_query(); }
  METAL_FUNC float3 get_committed_ray_origin() const thread { return __air_get_committed_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_committed_ray_direction() const thread { return __air_get_committed_ray_direction_intersection_query(); }

  METAL_FUNC void commit_triangle_intersection() thread { __air_commit_triangle_intersection_intersection_query(); }
  METAL_FUNC void commit_curve_intersection() thread { __air_commit_curve_intersection_intersection_query(); }
  METAL_FUNC void commit_bounding_box_intersection(float distance) thread {
    __air_commit_bounding_box_intersection_intersection_query(distance);
  }
  METAL_FUNC intersection_params get_intersection_params() const thread { return intersection_params(); }
  METAL_FUNC float3 get_world_space_ray_origin() const thread { return __air_get_world_space_ray_origin_intersection_query(); }
  METAL_FUNC float3 get_world_space_ray_direction() const thread { return __air_get_world_space_ray_direction_intersection_query(); }

  METAL_FUNC intersection_result<2> get() const thread { return intersection_result<2>(); }
};

// ============================================================================
// intersection_result world_space variants
// ============================================================================
template <>
struct intersection_result<5> {  // triangle world_space
  METAL_FUNC float3 get_world_space_origin() const thread { return __air_get_world_space_origin_intersection_result(); }
  METAL_FUNC float3 get_world_space_direction() const thread { return __air_get_world_space_direction_intersection_result(); }
  METAL_FUNC float get_world_space_min_distance() const thread { return __air_get_world_space_min_distance_intersection_result(); }
  METAL_FUNC float get_world_space_max_distance() const thread { return __air_get_world_space_max_distance_intersection_result(); }
  METAL_FUNC float3 get_object_space_origin() const thread { return __air_get_object_space_origin_intersection_result(); }
  METAL_FUNC float3 get_object_space_direction() const thread { return __air_get_object_space_direction_intersection_result(); }
  METAL_FUNC float get_object_space_min_distance() const thread { return __air_get_object_space_min_distance_intersection_result(); }
  METAL_FUNC float get_object_space_max_distance() const thread { return __air_get_object_space_max_distance_intersection_result(); }
  METAL_FUNC uint get_instance_id() const thread { return __air_get_instance_id_intersection_result(); }
  METAL_FUNC uint get_user_instance_id() const thread { return __air_get_user_instance_id_intersection_result(); }
  METAL_FUNC float4x3 get_world_to_object_transform() const thread { return __air_get_world_to_object_transform_intersection_result(); }
  METAL_FUNC float4x3 get_object_to_world_transform() const thread { return __air_get_object_to_world_transform_intersection_result(); }
  METAL_FUNC intersection_type get_type() const thread { return (intersection_type)__air_get_type_intersection_result(); }
  METAL_FUNC float get_distance() const thread { return __air_get_distance_intersection_result(); }
  METAL_FUNC uint get_geometry_id() const thread { return __air_get_geometry_id_intersection_result(); }
  METAL_FUNC uint get_primitive_id() const thread { return __air_get_primitive_id_intersection_result(); }
  METAL_FUNC bool is_triangle_front_facing() const thread { return __air_is_triangle_front_facing_intersection_result(); }
  METAL_FUNC float2 get_triangle_barycentric_coord() const thread { return __air_get_triangle_barycentric_coord_intersection_result(); }
};

// ============================================================================
// Additional visible_function_table specializations and methods
// ============================================================================
template <typename T>
struct visible_function_table<T*> {
  METAL_FUNC visible_function_table() thread = default;
  METAL_FUNC visible_function_table() device = default;
  METAL_FUNC visible_function_table() constant = default;
  METAL_FUNC visible_function_table() ray_data = default;

  METAL_FUNC visible_function_table(const thread visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const device visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const device coherent(device) visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const constant visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const ray_data visible_function_table &) thread = default;
  METAL_FUNC visible_function_table(const object_data visible_function_table &) thread = default;

  METAL_FUNC visible_function_table(const thread visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const device visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const device coherent(device) visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const constant visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const ray_data visible_function_table &) device = default;
  METAL_FUNC visible_function_table(const object_data visible_function_table &) device = default;

  METAL_FUNC visible_function_table(const thread visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const device visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const device coherent(device) visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const constant visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const ray_data visible_function_table &) constant = default;
  METAL_FUNC visible_function_table(const object_data visible_function_table &) constant = default;

  METAL_FUNC visible_function_table(const thread visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const device visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const device coherent(device) visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const constant visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const ray_data visible_function_table &) ray_data = default;
  METAL_FUNC visible_function_table(const object_data visible_function_table &) ray_data = default;

  // Assignment operators for all AS pairs
  METAL_FUNC thread visible_function_table &operator=(const thread visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const device visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const device coherent(device) visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const constant visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const ray_data visible_function_table &) thread = default;
  METAL_FUNC thread visible_function_table &operator=(const object_data visible_function_table &) thread = default;
  METAL_FUNC device visible_function_table &operator=(const thread visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const device visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const device coherent(device) visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const constant visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const ray_data visible_function_table &) device = default;
  METAL_FUNC device visible_function_table &operator=(const object_data visible_function_table &) device = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const thread visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const device visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const device coherent(device) visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const constant visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const ray_data visible_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) visible_function_table &operator=(const object_data visible_function_table &) device coherent(device) = default;
  METAL_FUNC constant visible_function_table &operator=(const thread visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const device visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const device coherent(device) visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const constant visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const ray_data visible_function_table &) constant = default;
  METAL_FUNC constant visible_function_table &operator=(const object_data visible_function_table &) constant = default;
  METAL_FUNC ray_data visible_function_table &operator=(const thread visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const device visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const device coherent(device) visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const constant visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const ray_data visible_function_table &) ray_data = default;
  METAL_FUNC ray_data visible_function_table &operator=(const object_data visible_function_table &) ray_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const thread visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const device visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const device coherent(device) visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const constant visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const ray_data visible_function_table &) object_data = default;
  METAL_FUNC object_data visible_function_table &operator=(const object_data visible_function_table &) object_data = default;

  METAL_FUNC bool empty() const thread { return __air_is_null_visible_function_table(t); }
  METAL_FUNC uint size() const thread { return __air_get_size_visible_function_table(t); }
  METAL_FUNC function_handle get_function_handle(uint index) const thread {
    return __air_get_function_handle_visible_function_table(t, index);
  }
private:
  __air_visible_function_table_t t;
};

// ============================================================================
// Additional intersection_function_table specializations
// ============================================================================
template <>
struct intersection_function_table<> {
  METAL_FUNC intersection_function_table() thread = default;
  METAL_FUNC intersection_function_table() device = default;
  METAL_FUNC intersection_function_table() constant = default;
  METAL_FUNC intersection_function_table() ray_data = default;
  METAL_FUNC intersection_function_table() object_data = default;

  METAL_FUNC intersection_function_table(const thread intersection_function_table &) thread = default;
  METAL_FUNC intersection_function_table(const device intersection_function_table &) thread = default;
  METAL_FUNC intersection_function_table(const device coherent(device) intersection_function_table &) thread = default;
  METAL_FUNC intersection_function_table(const constant intersection_function_table &) thread = default;
  METAL_FUNC intersection_function_table(const ray_data intersection_function_table &) thread = default;
  METAL_FUNC intersection_function_table(const object_data intersection_function_table &) thread = default;
  METAL_FUNC intersection_function_table(const thread intersection_function_table &) device = default;
  METAL_FUNC intersection_function_table(const device intersection_function_table &) device = default;
  METAL_FUNC intersection_function_table(const device coherent(device) intersection_function_table &) device = default;
  METAL_FUNC intersection_function_table(const constant intersection_function_table &) device = default;
  METAL_FUNC intersection_function_table(const ray_data intersection_function_table &) device = default;
  METAL_FUNC intersection_function_table(const object_data intersection_function_table &) device = default;
  METAL_FUNC intersection_function_table(const thread intersection_function_table &) constant = default;
  METAL_FUNC intersection_function_table(const device intersection_function_table &) constant = default;
  METAL_FUNC intersection_function_table(const device coherent(device) intersection_function_table &) constant = default;
  METAL_FUNC intersection_function_table(const constant intersection_function_table &) constant = default;
  METAL_FUNC intersection_function_table(const ray_data intersection_function_table &) constant = default;
  METAL_FUNC intersection_function_table(const object_data intersection_function_table &) constant = default;
  METAL_FUNC intersection_function_table(const thread intersection_function_table &) ray_data = default;
  METAL_FUNC intersection_function_table(const device intersection_function_table &) ray_data = default;
  METAL_FUNC intersection_function_table(const device coherent(device) intersection_function_table &) ray_data = default;
  METAL_FUNC intersection_function_table(const constant intersection_function_table &) ray_data = default;
  METAL_FUNC intersection_function_table(const ray_data intersection_function_table &) ray_data = default;
  METAL_FUNC intersection_function_table(const object_data intersection_function_table &) ray_data = default;
  METAL_FUNC intersection_function_table(const thread intersection_function_table &) object_data = default;
  METAL_FUNC intersection_function_table(const device intersection_function_table &) object_data = default;
  METAL_FUNC intersection_function_table(const device coherent(device) intersection_function_table &) object_data = default;
  METAL_FUNC intersection_function_table(const constant intersection_function_table &) object_data = default;
  METAL_FUNC intersection_function_table(const ray_data intersection_function_table &) object_data = default;
  METAL_FUNC intersection_function_table(const object_data intersection_function_table &) object_data = default;

  // Assignment operators
  METAL_FUNC thread intersection_function_table &operator=(const thread intersection_function_table &) thread = default;
  METAL_FUNC thread intersection_function_table &operator=(const device intersection_function_table &) thread = default;
  METAL_FUNC thread intersection_function_table &operator=(const device coherent(device) intersection_function_table &) thread = default;
  METAL_FUNC thread intersection_function_table &operator=(const constant intersection_function_table &) thread = default;
  METAL_FUNC thread intersection_function_table &operator=(const ray_data intersection_function_table &) thread = default;
  METAL_FUNC thread intersection_function_table &operator=(const object_data intersection_function_table &) thread = default;
  METAL_FUNC device intersection_function_table &operator=(const thread intersection_function_table &) device = default;
  METAL_FUNC device intersection_function_table &operator=(const device intersection_function_table &) device = default;
  METAL_FUNC device intersection_function_table &operator=(const device coherent(device) intersection_function_table &) device = default;
  METAL_FUNC device intersection_function_table &operator=(const constant intersection_function_table &) device = default;
  METAL_FUNC device intersection_function_table &operator=(const ray_data intersection_function_table &) device = default;
  METAL_FUNC device intersection_function_table &operator=(const object_data intersection_function_table &) device = default;
  METAL_FUNC device coherent(device) intersection_function_table &operator=(const thread intersection_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) intersection_function_table &operator=(const device intersection_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) intersection_function_table &operator=(const device coherent(device) intersection_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) intersection_function_table &operator=(const constant intersection_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) intersection_function_table &operator=(const ray_data intersection_function_table &) device coherent(device) = default;
  METAL_FUNC device coherent(device) intersection_function_table &operator=(const object_data intersection_function_table &) device coherent(device) = default;
  METAL_FUNC constant intersection_function_table &operator=(const thread intersection_function_table &) constant = default;
  METAL_FUNC constant intersection_function_table &operator=(const device intersection_function_table &) constant = default;
  METAL_FUNC constant intersection_function_table &operator=(const device coherent(device) intersection_function_table &) constant = default;
  METAL_FUNC constant intersection_function_table &operator=(const constant intersection_function_table &) constant = default;
  METAL_FUNC constant intersection_function_table &operator=(const ray_data intersection_function_table &) constant = default;
  METAL_FUNC constant intersection_function_table &operator=(const object_data intersection_function_table &) constant = default;
  METAL_FUNC ray_data intersection_function_table &operator=(const thread intersection_function_table &) ray_data = default;
  METAL_FUNC ray_data intersection_function_table &operator=(const device intersection_function_table &) ray_data = default;
  METAL_FUNC ray_data intersection_function_table &operator=(const device coherent(device) intersection_function_table &) ray_data = default;
  METAL_FUNC ray_data intersection_function_table &operator=(const constant intersection_function_table &) ray_data = default;
  METAL_FUNC ray_data intersection_function_table &operator=(const ray_data intersection_function_table &) ray_data = default;
  METAL_FUNC ray_data intersection_function_table &operator=(const object_data intersection_function_table &) ray_data = default;
  METAL_FUNC object_data intersection_function_table &operator=(const thread intersection_function_table &) object_data = default;
  METAL_FUNC object_data intersection_function_table &operator=(const device intersection_function_table &) object_data = default;
  METAL_FUNC object_data intersection_function_table &operator=(const device coherent(device) intersection_function_table &) object_data = default;
  METAL_FUNC object_data intersection_function_table &operator=(const constant intersection_function_table &) object_data = default;
  METAL_FUNC object_data intersection_function_table &operator=(const ray_data intersection_function_table &) object_data = default;
  METAL_FUNC object_data intersection_function_table &operator=(const object_data intersection_function_table &) object_data = default;

  METAL_FUNC bool empty() const thread { return __air_is_null_intersection_function_table(t); }
  METAL_FUNC uint size() const thread { return __air_get_size_intersection_function_table(t); }
  METAL_FUNC void set_buffer(const void *buf, uint index) thread { __air_set_buffer_intersection_function_table(t, buf, index); }
  METAL_FUNC void set_buffer(void *buf, uint index) thread { __air_set_buffer_intersection_function_table(t, buf, index); }
  template <typename VFT>
  METAL_FUNC void set_visible_function_table(visible_function_table<VFT> vft, uint index) thread {
    __air_set_visible_function_table_intersection_function_table(t, vft, index);
  }
  METAL_FUNC void set_function_handle_intersection_function_table(function_handle fh, uint index) thread {
    __air_set_function_handle_intersection_function_table(t, fh, index);
  }
  METAL_FUNC function_handle get_buffer_intersection_function_table(uint index) const thread {
    return __air_get_buffer_intersection_function_table(t, index);
  }
private:
  __air_intersection_function_table_t t;
};

// ============================================================================
// function_handle with address-space constructors
// ============================================================================
struct function_handle {
  METAL_FUNC function_handle() thread = default;
  METAL_FUNC function_handle() device = default;
  METAL_FUNC function_handle() constant = default;
  METAL_FUNC function_handle() ray_data = default;
  METAL_FUNC function_handle() object_data = default;

  METAL_FUNC function_handle(const thread function_handle &) thread = default;
  METAL_FUNC function_handle(const device function_handle &) thread = default;
  METAL_FUNC function_handle(const device coherent(device) function_handle &) thread = default;
  METAL_FUNC function_handle(const constant function_handle &) thread = default;
  METAL_FUNC function_handle(const ray_data function_handle &) thread = default;
  METAL_FUNC function_handle(const object_data function_handle &) thread = default;
  METAL_FUNC function_handle(const thread function_handle &) device = default;
  METAL_FUNC function_handle(const device function_handle &) device = default;
  METAL_FUNC function_handle(const device coherent(device) function_handle &) device = default;
  METAL_FUNC function_handle(const constant function_handle &) device = default;
  METAL_FUNC function_handle(const ray_data function_handle &) device = default;
  METAL_FUNC function_handle(const object_data function_handle &) device = default;
  METAL_FUNC function_handle(const thread function_handle &) constant = default;
  METAL_FUNC function_handle(const device function_handle &) constant = default;
  METAL_FUNC function_handle(const device coherent(device) function_handle &) constant = default;
  METAL_FUNC function_handle(const constant function_handle &) constant = default;
  METAL_FUNC function_handle(const ray_data function_handle &) constant = default;
  METAL_FUNC function_handle(const object_data function_handle &) constant = default;
  METAL_FUNC function_handle(const thread function_handle &) ray_data = default;
  METAL_FUNC function_handle(const device function_handle &) ray_data = default;
  METAL_FUNC function_handle(const device coherent(device) function_handle &) ray_data = default;
  METAL_FUNC function_handle(const constant function_handle &) ray_data = default;
  METAL_FUNC function_handle(const ray_data function_handle &) ray_data = default;
  METAL_FUNC function_handle(const object_data function_handle &) ray_data = default;
  METAL_FUNC function_handle(const thread function_handle &) object_data = default;
  METAL_FUNC function_handle(const device function_handle &) object_data = default;
  METAL_FUNC function_handle(const device coherent(device) function_handle &) object_data = default;
  METAL_FUNC function_handle(const constant function_handle &) object_data = default;
  METAL_FUNC function_handle(const ray_data function_handle &) object_data = default;
  METAL_FUNC function_handle(const object_data function_handle &) object_data = default;

  // Assignment operators
  METAL_FUNC thread function_handle &operator=(const thread function_handle &) thread = default;
  METAL_FUNC thread function_handle &operator=(const device function_handle &) thread = default;
  METAL_FUNC thread function_handle &operator=(const device coherent(device) function_handle &) thread = default;
  METAL_FUNC thread function_handle &operator=(const constant function_handle &) thread = default;
  METAL_FUNC thread function_handle &operator=(const ray_data function_handle &) thread = default;
  METAL_FUNC thread function_handle &operator=(const object_data function_handle &) thread = default;
  METAL_FUNC device function_handle &operator=(const thread function_handle &) device = default;
  METAL_FUNC device function_handle &operator=(const device function_handle &) device = default;
  METAL_FUNC device function_handle &operator=(const device coherent(device) function_handle &) device = default;
  METAL_FUNC device function_handle &operator=(const constant function_handle &) device = default;
  METAL_FUNC device function_handle &operator=(const ray_data function_handle &) device = default;
  METAL_FUNC device function_handle &operator=(const object_data function_handle &) device = default;
  METAL_FUNC device coherent(device) function_handle &operator=(const thread function_handle &) device coherent(device) = default;
  METAL_FUNC device coherent(device) function_handle &operator=(const device function_handle &) device coherent(device) = default;
  METAL_FUNC device coherent(device) function_handle &operator=(const device coherent(device) function_handle &) device coherent(device) = default;
  METAL_FUNC device coherent(device) function_handle &operator=(const constant function_handle &) device coherent(device) = default;
  METAL_FUNC device coherent(device) function_handle &operator=(const ray_data function_handle &) device coherent(device) = default;
  METAL_FUNC device coherent(device) function_handle &operator=(const object_data function_handle &) device coherent(device) = default;
  METAL_FUNC constant function_handle &operator=(const thread function_handle &) constant = default;
  METAL_FUNC constant function_handle &operator=(const device function_handle &) constant = default;
  METAL_FUNC constant function_handle &operator=(const device coherent(device) function_handle &) constant = default;
  METAL_FUNC constant function_handle &operator=(const constant function_handle &) constant = default;
  METAL_FUNC constant function_handle &operator=(const ray_data function_handle &) constant = default;
  METAL_FUNC constant function_handle &operator=(const object_data function_handle &) constant = default;
  METAL_FUNC ray_data function_handle &operator=(const thread function_handle &) ray_data = default;
  METAL_FUNC ray_data function_handle &operator=(const device function_handle &) ray_data = default;
  METAL_FUNC ray_data function_handle &operator=(const device coherent(device) function_handle &) ray_data = default;
  METAL_FUNC ray_data function_handle &operator=(const constant function_handle &) ray_data = default;
  METAL_FUNC ray_data function_handle &operator=(const ray_data function_handle &) ray_data = default;
  METAL_FUNC ray_data function_handle &operator=(const object_data function_handle &) ray_data = default;
  METAL_FUNC object_data function_handle &operator=(const thread function_handle &) object_data = default;
  METAL_FUNC object_data function_handle &operator=(const device function_handle &) object_data = default;
  METAL_FUNC object_data function_handle &operator=(const device coherent(device) function_handle &) object_data = default;
  METAL_FUNC object_data function_handle &operator=(const constant function_handle &) object_data = default;
  METAL_FUNC object_data function_handle &operator=(const ray_data function_handle &) object_data = default;
  METAL_FUNC object_data function_handle &operator=(const object_data function_handle &) object_data = default;

  METAL_FUNC bool empty() const thread { return __air_is_null_function_handle(t); }
  METAL_FUNC function_handle get_function_handle_intersection_function_table() const thread {
    return __air_get_function_handle_intersection_function_table(t);
  }
private:
  __air_function_handle_t t;
};

// ============================================================================
// Additional intersection_result world-space variants (gap closure)
// ============================================================================
template <>
struct intersection_result<6> {  // multi-level triangle world_space
  METAL_FUNC float3 get_world_space_origin() const thread { return __air_get_world_space_origin_intersection_result(); }
  METAL_FUNC float3 get_world_space_direction() const thread { return __air_get_world_space_direction_intersection_result(); }
  METAL_FUNC float get_world_space_min_distance() const thread { return __air_get_world_space_min_distance_intersection_result(); }
  METAL_FUNC float get_world_space_max_distance() const thread { return __air_get_world_space_max_distance_intersection_result(); }
  METAL_FUNC float3 get_object_space_origin() const thread { return __air_get_object_space_origin_intersection_result(); }
  METAL_FUNC float3 get_object_space_direction() const thread { return __air_get_object_space_direction_intersection_result(); }
  METAL_FUNC float get_object_space_min_distance() const thread { return __air_get_object_space_min_distance_intersection_result(); }
  METAL_FUNC float get_object_space_max_distance() const thread { return __air_get_object_space_max_distance_intersection_result(); }
  METAL_FUNC uint get_instance_count() const thread { return __air_get_instance_count_intersection_result(); }
  METAL_FUNC uint get_instance_id() const thread { return __air_get_instance_id_intersection_result(); }
  METAL_FUNC uint get_user_instance_id() const thread { return __air_get_user_instance_id_intersection_result(); }
  METAL_FUNC uint get_instance_id(uint depth) const thread { return __air_get_instance_id_depth_intersection_result(depth); }
  METAL_FUNC uint get_user_instance_id(uint depth) const thread { return __air_get_user_instance_id_depth_intersection_result(depth); }
  METAL_FUNC float4x3 get_world_to_object_transform() const thread { return __air_get_world_to_object_transform_intersection_result(); }
  METAL_FUNC float4x3 get_object_to_world_transform() const thread { return __air_get_object_to_world_transform_intersection_result(); }
  METAL_FUNC intersection_type get_type() const thread { return (intersection_type)__air_get_type_intersection_result(); }
  METAL_FUNC float get_distance() const thread { return __air_get_distance_intersection_result(); }
  METAL_FUNC uint get_geometry_id() const thread { return __air_get_geometry_id_intersection_result(); }
  METAL_FUNC uint get_primitive_id() const thread { return __air_get_primitive_id_intersection_result(); }
  METAL_FUNC bool is_triangle_front_facing() const thread { return __air_is_triangle_front_facing_intersection_result(); }
  METAL_FUNC float2 get_triangle_barycentric_coord() const thread { return __air_get_triangle_barycentric_coord_intersection_result(); }
};

using intersection_result_triangle_world_space = intersection_result<5>;
using intersection_result_triangle_instancing_world_space = intersection_result<6>;

// Additional helper for null intersection function table
METAL_FUNC bool is_null_intersection_function_table_default() { return __air_is_null_intersection_function_table_default(); }
