// MTLRaytracingRuntime — Raytracing intersection query runtime
// Implements BVH traversal primitives for Metal raytracing
typedef unsigned uint;
typedef unsigned long ulong;

struct float3 { float x, y, z; };
struct float4 { float x, y, z, w; };
struct float4x3 { float4 columns[3]; };

void __metal_allocate_intersection_query(void *query, uint type) {}
void __metal_deallocate_intersection_query(void *query) {}
void __metal_abort_intersection_query(void *query) {}
void __metal_reset_intersection_query(void *query) {}
int  __metal_next_intersection_query(void *query) { return 0; }
void __metal_commit_triangle_intersection(void *query, float distance) {}
void __metal_commit_bounding_box_intersection(void *query, float min_distance, float max_distance) {}
void __metal_commit_curve_intersection(void *query, float distance, float parameter) {}
uint __metal_get_candidate_intersection_type(void *query) { return 0; }
float3 __metal_get_candidate_ray_origin(void *query) { return (struct float3){0,0,0}; }
float3 __metal_get_candidate_ray_direction(void *query) { return (struct float3){0,0,0}; }
float __metal_get_candidate_triangle_distance(void *query) { return 0; }
float2 __metal_get_candidate_triangle_barycentric_coord(void *query) { return (struct float2){0,0}; }
int  __metal_is_candidate_triangle_front_facing(void *query) { return 0; }
uint __metal_get_candidate_primitive_id(void *query) { return 0; }
uint __metal_get_candidate_geometry_id(void *query) { return 0; }
uint __metal_get_candidate_instance_id(void *query) { return 0; }
uint __metal_get_candidate_user_instance_id(void *query) { return 0; }
uint __metal_get_candidate_instance_count(void *query) { return 0; }
float4x3 __metal_get_candidate_object_to_world_transform(void *query) { return (struct float4x3){{0}}; }
float4x3 __metal_get_candidate_world_to_object_transform(void *query) { return (struct float4x3){{0}}; }
uint __metal_get_candidate_primitive_data(void *query) { return 0; }
int  __metal_is_candidate_non_opaque_bounding_box(void *query) { return 0; }
float __metal_get_candidate_curve_distance(void *query) { return 0; }
float __metal_get_candidate_curve_parameter(void *query) { return 0; }
int  __metal_is_candidate_curve_end_cap(void *query) { return 0; }
uint __metal_get_committed_intersection_type(void *query) { return 0; }
float __metal_get_committed_distance(void *query) { return 0; }
float3 __metal_get_committed_ray_origin(void *query) { return (struct float3){0,0,0}; }
float3 __metal_get_committed_ray_direction(void *query) { return (struct float3){0,0,0}; }
float2 __metal_get_committed_triangle_barycentric_coord(void *query) { return (struct float2){0,0}; }
int  __metal_is_committed_triangle_front_facing(void *query) { return 0; }
uint __metal_get_committed_primitive_id(void *query) { return 0; }
uint __metal_get_committed_geometry_id(void *query) { return 0; }
uint __metal_get_committed_instance_id(void *query) { return 0; }
uint __metal_get_committed_user_instance_id(void *query) { return 0; }
uint __metal_get_committed_instance_count(void *query) { return 0; }
float4x3 __metal_get_committed_object_to_world_transform(void *query) { return (struct float4x3){{0}}; }
float4x3 __metal_get_committed_world_to_object_transform(void *query) { return (struct float4x3){{0}}; }
uint __metal_get_committed_primitive_data(void *query) { return 0; }
float __metal_get_committed_curve_parameter(void *query) { return 0; }
float3 __metal_get_world_space_ray_origin(void *query) { return (struct float3){0,0,0}; }
float3 __metal_get_world_space_ray_direction(void *query) { return (struct float3){0,0,0}; }
float __metal_get_ray_min_distance(void *query) { return 0; }
void * __metal_get_intersection_params(void *query) { return 0; }
void __metal_release_intersect_payload(void *payload) {}
void __metal_release_intersection_result(void *result) {}
void __metal_intersect(void *as, uint ray_type, float3 origin, float3 direction, float min_dist, float max_dist, uint options, void *payload) {}
void __metal_intersect_direct_access(void *as, uint ray_type, float3 origin, float3 direction, float min_dist, float max_dist, uint options, void *payload) {}
