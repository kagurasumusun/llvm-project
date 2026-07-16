// target.cpp - Metal Target Platform Information
// Provides platform detection for Metal runtime

extern "C" {

// Platform identification
const char* ___metal_platform_name_macos = "macos";
const char* ___metal_platform_name_ios = "ios";
const char* ___metal_platform_name_tvos = "tvos";
const char* ___metal_platform_name_watchos = "watchos";
const char* ___metal_platform_name_xros = "xros";

// Metal version constants
const int ___metal_version_1_0 = 100;
const int ___metal_version_1_1 = 110;
const int ___metal_version_1_2 = 120;
const int ___metal_version_2_0 = 200;
const int ___metal_version_2_1 = 210;
const int ___metal_version_2_2 = 220;
const int ___metal_version_2_3 = 230;
const int ___metal_version_2_4 = 240;
const int ___metal_version_3_0 = 300;
const int ___metal_version_3_1 = 310;
const int ___metal_version_4_0 = 400;
const int ___metal_version_4_1 = 410;

// GPU family constants
const int ___metal_gpu_family_apple1 = 1001;
const int ___metal_gpu_family_apple2 = 1002;
const int ___metal_gpu_family_apple3 = 1003;
const int ___metal_gpu_family_apple4 = 1004;
const int ___metal_gpu_family_apple5 = 1005;
const int ___metal_gpu_family_apple6 = 1006;
const int ___metal_gpu_family_apple7 = 1007;
const int ___metal_gpu_family_apple8 = 1008;
const int ___metal_gpu_family_apple9 = 1009;

// Device caps
const int ___metal_max_threads_per_threadgroup = 1024;
const int ___metal_max_threadgroup_memory = 32768;
const int ___metal_max_buffer_arguments = 31;
const int ___metal_max_texture_arguments = 128;
const int ___metal_max_sampler_arguments = 16;

} // extern C
