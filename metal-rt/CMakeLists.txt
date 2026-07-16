# Metal Runtime Library - Clean Room Implementation
# Builds libmetal-rt.a for all Apple platforms
cmake_minimum_required(VERSION 3.20)
project(metal-rt CXX)

set(METAL_RT_SOURCES
  metal_runtime_full.cpp
  metal_math.cpp
  metal_integer.cpp
  metal_cstring.cpp
  metal_logging.cpp
  metal_vector.cpp
  metal_texture.cpp
  metal_sync.cpp
  metal_atomic.cpp
  metal_matrix.cpp
  metal_geometric.cpp
  metal_relational.cpp
  metal_common.cpp
  metal_half.cpp
  metal_bfloat.cpp
  target.cpp
)

# Build for each Apple platform
foreach(platform macos ios tvos watchos xros)
  add_library(metal-rt-${platform} STATIC ${METAL_RT_SOURCES})
  set_target_properties(metal-rt-${platform} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
  )
  target_compile_options(metal-rt-${platform} PRIVATE
    -fno-exceptions -fno-rtti -nostdinc++ -nostdlib
    -ffreestanding -fno-builtin
  )
endforeach()
