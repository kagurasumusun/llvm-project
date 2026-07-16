// metal_relational.cpp - Metal Relational Functions

extern "C" {

// isequal, isnotequal, isgreater, isgreaterequal, isless, islessequal
bool ___metal_isequal_float(float a, float b) { return a == b; }
bool ___metal_isequal_double(double a, double b) { return a == b; }
bool ___metal_isnotequal_float(float a, float b) { return a != b; }
bool ___metal_isgreater_float(float a, float b) { return a > b; }
bool ___metal_isgreaterequal_float(float a, float b) { return a >= b; }
bool ___metal_isless_float(float a, float b) { return a < b; }
bool ___metal_islessequal_float(float a, float b) { return a <= b; }
bool ___metal_islessgreater_float(float a, float b) { return (a < b) || (a > b); }

// isnan, isinf, isfinite, isnormal
bool ___metal_isnan_float(float x) { return __builtin_isnan(x); }
bool ___metal_isnan_double(double x) { return __builtin_isnan(x); }
bool ___metal_isinf_float(float x) { return __builtin_isinf(x); }
bool ___metal_isinf_double(double x) { return __builtin_isinf(x); }
bool ___metal_isfinite_float(float x) { return __builtin_isfinite(x); }
bool ___metal_isfinite_double(double x) { return __builtin_isfinite(x); }
bool ___metal_isnormal_float(float x) { return __builtin_isnormal(x); }
bool ___metal_isnormal_double(double x) { return __builtin_isnormal(x); }
bool ___metal_signbit_float(float x) { return __builtin_signbit(x); }
bool ___metal_signbit_double(double x) { return __builtin_signbit(x); }

// any, all, select
bool ___metal_any_bool(bool x) { return x; }
bool ___metal_any_v2bool(bool x, bool y) { return x || y; }
bool ___metal_any_v3bool(bool x, bool y, bool z) { return x || y || z; }
bool ___metal_any_v4bool(bool x, bool y, bool z, bool w) { return x || y || z || w; }
bool ___metal_all_bool(bool x) { return x; }
bool ___metal_all_v2bool(bool x, bool y) { return x && y; }
bool ___metal_all_v3bool(bool x, bool y, bool z) { return x && y && z; }
bool ___metal_all_v4bool(bool x, bool y, bool z, bool w) { return x && y && z && w; }

} // extern C
