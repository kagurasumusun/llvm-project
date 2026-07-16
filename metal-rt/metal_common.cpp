// metal_common.cpp - Metal Common Functions

extern "C" {

// clamp
float ___metal_clamp_float_float_float(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }
double ___metal_clamp_double_double_double(double x, double lo, double hi) { return x < lo ? lo : (x > hi ? hi : x); }
float ___metal_clamp_half_half_half(float x, float lo, float hi) { return x < lo ? lo : (x > hi ? hi : x); }

// saturate
float ___metal_saturate_float(float x) { return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }
double ___metal_saturate_double(double x) { return x < 0.0 ? 0.0 : (x > 1.0 ? 1.0 : x); }

// mix
float ___metal_mix_float_float_float(float x, float y, float a) { return x + (y - x) * a; }
double ___metal_mix_double_double_double(double x, double y, double a) { return x + (y - x) * a; }

// step
float ___metal_step_float_float(float edge, float x) { return x < edge ? 0.0f : 1.0f; }
double ___metal_step_double_double(double edge, double x) { return x < edge ? 0.0 : 1.0; }

// smoothstep
float ___metal_smoothstep_float_float_float(float edge0, float edge1, float x) {
    float t = (x - edge0) / (edge1 - edge0);
    t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    return t * t * (3.0f - 2.0f * t);
}
double ___metal_smoothstep_double_double_double(double edge0, double edge1, double x) {
    double t = (x - edge0) / (edge1 - edge0);
    t = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
    return t * t * (3.0 - 2.0 * t);
}

// sign
float ___metal_sign_float(float x) { return (x > 0) ? 1.0f : ((x < 0) ? -1.0f : 0.0f); }
double ___metal_sign_double(double x) { return (x > 0) ? 1.0 : ((x < 0) ? -1.0 : 0.0); }

// degrees, radians
float ___metal_degrees_float(float x) { return x * 57.29577951308232f; }
float ___metal_radians_float(float x) { return x * 0.01745329251994f; }
double ___metal_degrees_double(double x) { return x * 57.29577951308232; }
double ___metal_radians_double(double x) { return x * 0.01745329251994; }

// M_PI constants
const float ___metal_M_PI_F = 3.14159265358979323846f;
const double ___metal_M_PI = 3.14159265358979323846;
const float ___metal_M_PI_2_F = 1.5707963267948966f;
const float ___metal_M_PI_4_F = 0.7853981633974483f;
const float ___metal_M_1_PI_F = 0.3183098861837907f;
const float ___metal_M_2_PI_F = 0.6366197723675813f;
const float ___metal_M_2_SQRTPI_F = 1.1283791670955126f;
const float ___metal_M_SQRT2_F = 1.4142135623730951f;
const float ___metal_M_SQRT1_2_F = 0.7071067811865475f;
const float ___metal_M_E_F = 2.718281828459045f;
const float ___metal_M_LOG2E_F = 1.4426950408889634f;
const float ___metal_M_LOG10E_F = 0.4342944819032518f;
const float ___metal_M_LN2_F = 0.6931471805599453f;
const float ___metal_M_LN10_F = 2.302585092994046f;
const float ___metal_MAXFLOAT_F = 3.4028234663852886e+38f;
const float ___metal_HUGE_VALF = __builtin_huge_valf();
const float ___metal_INFINITY_F = __builtin_inff();
const float ___metal_NAN_F = __builtin_nanf("");

} // extern C
