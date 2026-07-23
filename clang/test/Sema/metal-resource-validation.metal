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
