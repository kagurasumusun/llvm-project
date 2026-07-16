// metal_vector.cpp - Complete Metal Vector Functions
// Clean-room implementation

extern "C" {

// ============================================================================
// dot, cross, distance, length, normalize, fast_length, fast_distance, fast_normalize
// ============================================================================

// dot products for all vector sizes
float ___metal_dot_v2float(float x1, float x2, float y1, float y2) { return x1*y1 + x2*y2; }
float ___metal_dot_v3float(float x1, float x2, float x3, float y1, float y2, float y3) { return x1*y1 + x2*y2 + x3*y3; }
float ___metal_dot_v4float(float x1, float x2, float x3, float x4, float y1, float y2, float y3, float y4) { return x1*y1 + x2*y2 + x3*y3 + x4*y4; }
double ___metal_dot_v2double(double x1, double x2, double y1, double y2) { return x1*y1 + x2*y2; }
double ___metal_dot_v3double(double x1, double x2, double y3, double y1, double y2, double y3b) { return x1*y1 + x2*y2 + y3*y3b; }
double ___metal_dot_v4double(double x1, double x2, double x3, double x4, double y1, double y2, double y3, double y4) { return x1*y1 + x2*y2 + x3*y3 + x4*y4; }

// cross product (3D only)
float ___metal_cross_v3float(float ax, float ay, float az, float bx, float by, float bz) {
    return ay*bz - az*by;
}
double ___metal_cross_v3double(double ax, double ay, double az, double bx, double by, double bz) {
    return ay*bz - az*by;
}

// length
float ___metal_length_v2float(float x, float y) { return __builtin_sqrtf(x*x + y*y); }
float ___metal_length_v3float(float x, float y, float z) { return __builtin_sqrtf(x*x + y*y + z*z); }
float ___metal_length_v4float(float x, float y, float z, float w) { return __builtin_sqrtf(x*x + y*y + z*z + w*w); }
float ___metal_length_float(float x) { return __builtin_fabsf(x); }
double ___metal_length_double(double x) { return __builtin_fabs(x); }

// distance
float ___metal_distance_v2float(float x1, float y1, float x2, float y2) { float dx=x1-x2, dy=y1-y2; return __builtin_sqrtf(dx*dx+dy*dy); }
float ___metal_distance_v3float(float x1, float y1, float z1, float x2, float y2, float z2) { float dx=x1-x2,dy=y1-y2,dz=z1-z2; return __builtin_sqrtf(dx*dx+dy*dy+dz*dz); }

// normalize
float ___metal_normalize_v2float(float x, float y) { float l = __builtin_sqrtf(x*x+y*y); return l > 0 ? x/l : 0; }

// fast_length, fast_distance, fast_normalize - half-precision versions
float ___metal_fast_length_v3float(float x, float y, float z) { return __builtin_sqrtf(x*x+y*y+z*z); }
float ___metal_fast_distance_v3float(float x1, float y1, float z1, float x2, float y2, float z2) { float dx=x1-x2,dy=y1-y2,dz=z1-z2; return __builtin_sqrtf(dx*dx+dy*dy+dz*dz); }

// faceforward
float ___metal_faceforward_v3float(float n, float ref) { return ref < 0 ? n : -n; }

// reflect
float ___metal_reflect_v3float(float i, float n) { return i - 2.0f * n * i; }

// refract
float ___metal_refract_v3float(float i, float n, float eta) {
    float d = n * i;
    float k = 1.0f - eta * eta * (1.0f - d * d);
    if (k < 0.0f) return 0.0f;
    return eta * i - (eta * d + __builtin_sqrtf(k)) * n;
}

// select
float ___metal_select_float_float_bool(float a, float b, bool c) { return c ? b : a; }
int32_t ___metal_select_int32_int32_bool(int32_t a, int32_t b, bool c) { return c ? b : a; }
uint32_t ___metal_select_uint32_uint32_bool(uint32_t a, uint32_t b, bool c) { return c ? b : a; }

} // extern C
