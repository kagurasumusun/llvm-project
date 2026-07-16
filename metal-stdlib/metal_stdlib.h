// metal_stdlib.h - Complete Metal Standard Library
// Clean-room implementation for Linux cross-compilation
// Copyright (c) 2026 Metal Linux Compiler Project
// SPDX-License-Identifier: MIT
// Supports Metal 1.0 through 4.1

#ifndef __METAL_STD_LIB_H__
#define __METAL_STD_LIB_H__

// Include all sub-headers
#include "metal_types.h"
#include "metal_math.h"
#include "metal_geometric.h"
#include "metal_matrix.h"
#include "metal_texture.h"
#include "metal_atomic.h"
#include "metal_sync.h"
#include "metal_relational.h"
#include "metal_common.h"
#include "metal_half.h"

// Metal namespace
namespace metal {

// Using declarations for all Metal types and functions
using namespace metal_types;
using namespace metal_math;
using namespace metal_geometric;
using namespace metal_matrix;
using namespace metal_texture;
using namespace metal_atomic;
using namespace metal_sync;
using namespace metal_relational;
using namespace metal_common;

} // namespace metal

// Metal 4.0+ concepts support
#if __cplusplus >= 202002L
#include <concepts>
namespace metal {
template<typename T> concept scalar_type = __is_scalar_type(T);
template<typename T> concept vector_type = requires { typename T::value_type; };
template<typename T> concept matrix_type = requires { typename T::column_type; };
} // namespace metal
#endif

// Metal 4.1 Mesh shader support
#if defined(METAL_VERSION) && METAL_VERSION >= 410
namespace metal {
template<typename T, int M, int N> struct mesh_grid;
template<typename T> struct mesh_thread;
template<typename T, int Size> struct mesh_input;
template<typename Payload, int Size> struct mesh;
template<typename Payload> struct object_data;
} // namespace metal
#endif

#endif // __METAL_STD_LIB_H__
