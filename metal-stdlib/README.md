# Metal Standard Library (metal-stdlib)

Complete clean-room implementation of Apple's Metal Standard Library headers.

## Files
- `metal_stdlib.h` - Main Metal standard library header
- `metal_types.h` - Metal type definitions
- `metal_math.h` - Math function declarations
- `metal_geometric.h` - Geometric function declarations
- `metal_matrix.h` - Matrix type definitions
- `metal_texture.h` - Texture type definitions
- `metal_atomic.h` - Atomic function declarations
- `metal_sync.h` - Synchronization primitives
- `metal_relational.h` - Relational function declarations
- `metal_common.h` - Common utility functions
- `metal_half.h` - Half-precision support
- `metal_bfloat.h` - BFloat16 support (Metal 4.0+)

## Metal Versions
Fully supports Metal 1.0 through 4.1, including all deprecated APIs.

## C++ Extensions
Includes optional C++14/17/20/23 extensions that don't modify the AIR/GPU model:
- constexpr support
- Concepts (Metal 4.1 equivalent)
- Structured bindings
- if constexpr
- auto with templates
