// MTLShaderLoggingRuntime — Shader logging runtime
// Enables os_log-style logging from GPU shaders
typedef unsigned uint;

void __metal_shader_log_begin(void *log_object, uint message_id) {}
void __metal_shader_log_end(void *log_object) {}
void __metal_shader_log_write_bool(void *log_object, int value) {}
void __metal_shader_log_write_int(void *log_object, int value) {}
void __metal_shader_log_write_uint(void *log_object, uint value) {}
void __metal_shader_log_write_float(void *log_object, float value) {}
void __metal_shader_log_write_string(void *log_object, const char *str) {}
void __metal_shader_log_write_float2(void *log_object, float x, float y) {}
void __metal_shader_log_write_float3(void *log_object, float x, float y, float z) {}
void __metal_shader_log_write_float4(void *log_object, float x, float y, float z, float w) {}
void __metal_shader_log_write_int2(void *log_object, int x, int y) {}
void __metal_shader_log_write_int3(void *log_object, int x, int y, int z) {}
void __metal_shader_log_write_int4(void *log_object, int x, int y, int z, int w) {}
void __metal_shader_log_write_uint2(void *log_object, uint x, uint y) {}
void __metal_shader_log_write_uint3(void *log_object, uint x, uint y, uint z) {}
void __metal_shader_log_write_uint4(void *log_object, uint x, uint y, uint z, uint w) {}
