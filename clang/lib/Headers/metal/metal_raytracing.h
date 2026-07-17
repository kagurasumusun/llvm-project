#ifndef __METAL_RAYTRACING_H__
#define __METAL_RAYTRACING_H__

#if 1 // defined(__METAL__) || defined(__metal__)
namespace metal {

namespace raytracing {

enum class intersection_type {
    none = 0,
    triangle = 1,
    bounding_box = 2
};

struct ray {
    float3 origin;
    float min_distance;
    float3 direction;
    float max_distance;
    ray() : min_distance(0.0f), max_distance(3.402823466e+38f) {}
    ray(float3 o, float3 d, float min_d = 0.0f, float max_d = 3.402823466e+38f)
        : origin(o), min_distance(min_d), direction(d), max_distance(max_d) {}
};

template<typename instantiating_classifier = void>
struct intersector {
    intersector() {}
    void assume_geometry_type(intersection_type type) {}
};

template<typename instantiating_classifier = void>
struct intersection_query {
    intersection_query() {}
    bool next() { return false; }
    intersection_type get_candidate_intersection_type() const { return intersection_type::none; }
    float get_candidate_triangle_distance() const { return 0.0f; }
    void commit_bounding_box_intersection(float distance) {}
    void commit_triangle_intersection() {}
    void abort() {}
};

struct acceleration_structure {
    acceleration_structure() {}
};

template<typename T = void>
struct intersection_function_table {
    intersection_function_table() {}
};

template<typename T = void>
struct visible_function_table {
    visible_function_table() {}
};

} // namespace raytracing

using raytracing::ray;
using raytracing::intersector;
using raytracing::intersection_query;
using raytracing::acceleration_structure;
using raytracing::intersection_function_table;
using raytracing::visible_function_table;
using raytracing::intersection_type;

} // namespace metal
#endif // __METAL__
#endif // __METAL_RAYTRACING_H__
