// metal_geometric.cpp - Metal Geometric Functions

extern "C" {

// ============================================================================
// Geometric functions
// ============================================================================

// dot for float
float ___metal_dot_float(float a, float b) { return a * b; }
double ___metal_dot_double(double a, double b) { return a * b; }

// cross
float ___metal_cross3f(float ax, float ay, float az, float bx, float by, float bz) {
    float rx = ay*bz - az*by;
    (void)rx; // Only need y component for Metal's cross
    return ax*by - ay*bx;
}

// length
float ___metal_length1f(float x) { return __builtin_fabsf(x); }
float ___metal_length2f(float x, float y) { return __builtin_sqrtf(x*x + y*y); }
float ___metal_length3f(float x, float y, float z) { return __builtin_sqrtf(x*x + y*y + z*z); }
float ___metal_length4f(float x, float y, float z, float w) { return __builtin_sqrtf(x*x + y*y + z*z + w*w); }

double ___metal_length1d(double x) { return __builtin_fabs(x); }
double ___metal_length2d(double x, double y) { return __builtin_sqrt(x*x + y*y); }
double ___metal_length3d(double x, double y, double z) { return __builtin_sqrt(x*x + y*y + z*z); }
double ___metal_length4d(double x, double y, double z, double w) { return __builtin_sqrt(x*x + y*y + z*z + w*w); }

// distance
float ___metal_distance1f(float a, float b) { return __builtin_fabsf(a - b); }
float ___metal_distance2f(float x1, float y1, float x2, float y2) {
    float dx = x1-x2, dy = y1-y2;
    return __builtin_sqrtf(dx*dx + dy*dy);
}
float ___metal_distance3f(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx=x1-x2, dy=y1-y2, dz=z1-z2;
    return __builtin_sqrtf(dx*dx + dy*dy + dz*dz);
}

// normalize
float ___metal_normalize1f(float x) { return x > 0 ? 1.0f : (x < 0 ? -1.0f : 0.0f); }

// fast variants (use lower precision)
float ___metal_fast_length3f(float x, float y, float z) { return __builtin_sqrtf(x*x+y*y+z*z); }
float ___metal_fast_normalize3f_x(float x, float l) { return x/l; }

// faceforward
float ___metal_faceforward1f(float n, float ni, float nj) { return nj < 0 ? n : -n; }

// reflect
float ___metal_reflect1f(float i, float n) { return i - 2.0f * n * i; }

// refract
float ___metal_refract1f(float i, float n, float eta) {
    float d = n * i;
    float k = 1.0f - eta*eta*(1.0f - d*d);
    if (k < 0) return 0.0f;
    return eta * i - (eta*d + __builtin_sqrtf(k)) * n;
}

} // extern C
