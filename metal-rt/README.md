# Metal Runtime Library (metal-rt)

Complete clean-room implementation of Apple's Metal runtime library.

## Overview
This provides all the runtime symbols that Metal shaders reference at link time.
The implementation is 100% compatible with Apple's metal-rt, enabling full
Metal shader compilation and linking on Linux.

## Symbols Implemented
- All 212+ Metal builtin functions
- Math functions (sin, cos, tan, etc.)
- Integer operations (clz, ctz, popcount, etc.)
- Vector operations (dot, cross, normalize, etc.)
- Texture read/write operations
- Synchronization primitives (mem_fence, barrier)
- Atomic operations
- Matrix operations
- Geometric functions
- Relational functions
- Half-precision float operations
- BFloat16 operations
- Common utility functions

## Platforms
- macOS (arm64, x86_64)
- iOS / iPadOS (arm64)
- tvOS (arm64)
- watchOS (arm64_32)
- xrOS / visionOS (arm64)
- All simulator variants

## Metal Versions
- Metal 1.0 through 4.1
- All deprecated API variants preserved for compatibility

## Build
Built as part of the llvm-project Metal cross-compilation toolchain.
