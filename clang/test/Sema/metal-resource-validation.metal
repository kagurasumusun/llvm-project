// RUN: %clang_cc1 -triple air64-apple-macosx26.0 -x metal -std=metal4.0 -fsyntax-only -verify %s

constant bool fc_bool [[function_constant(0)]];
constant int fc_int [[function_constant(1)]];
constant uint4 fc_vec [[function_constant(2)]]; // expected-error {{'function_constant' attribute requires parameter type a global scalar bool, integer, enum, or floating type}}

kernel void good_resources(device int *out [[buffer(0)]],
                           constant int &in [[buffer(1)]],
                           texture2d tex [[texture(0)]],
                           sampler samp [[sampler(0)]]) {}

kernel void bad_texture(device int *out [[texture(0)]]) {} // expected-error {{'texture' attribute requires parameter type a texture or depth texture object type}}
kernel void bad_sampler(texture2d tex [[sampler(0)]]) {} // expected-error {{'sampler' attribute requires parameter type a sampler object type}}
kernel void bad_buffer(texture2d tex [[buffer(0)]]) {} // expected-error {{'buffer' attribute requires parameter type a pointer or reference resource}}
kernel void bad_sampler_index(sampler samp [[sampler(16)]]) {} // expected-error {{'sampler' attribute requires integer constant between 0 and 15 inclusive}}
constant int fc_dup [[function_constant(1)]]; // expected-error {{'function_constant' attribute index 1 is already used by another function constant}}

kernel void bad_duplicate_buffers(device int *a [[buffer(2)]], device int *b [[buffer(2)]]) {} // expected-error {{'buffer' attribute index 2 is already used by another parameter}}
kernel void bad_duplicate_textures(texture2d a [[texture(3)]], texture2d b [[texture(3)]]) {} // expected-error {{'texture' attribute index 3 is already used by another parameter}}
kernel void bad_duplicate_samplers(sampler a [[sampler(4)]], sampler b [[sampler(4)]]) {} // expected-error {{'sampler' attribute index 4 is already used by another parameter}}
