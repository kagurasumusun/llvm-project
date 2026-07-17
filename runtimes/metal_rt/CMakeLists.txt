#===-- runtimes/metal_rt/CMakeLists.txt - Metal Runtime Build Setup -----===#
#
# Builds 100% clean-room native replacements for Metal platform SDK libraries:
#   - libmetal_rt_*.a
#   - libair_rt_*.rtlib
#   - libresource_tracking_rt_*.rtlib
#   - MTLShaderLoggingRuntime.rtlib
#   - MTLRaytracingRuntime.rtlib
#   - libpost_mesh_dump_rt_*.rtlib
#
# Preserves and copies official OpenCL, tracepoint, and workaround binaries:
#   - libopencl_rt_*.a
#   - libtracepoint_rt_static_*.a
#   - libtracepoint_rt_workaround_*.a
#   - libtracepoint_rt_*.metallib
#
#===----------------------------------------------------------------------===#

cmake_minimum_required(VERSION 3.20.0)

# 1. First, copy all official SDK resources to the build directory's lib folder
file(GLOB SDK_FILES "/home/jules/Downloads/Xcode26.5/Metal.xctoolchain/usr/metal/32023/lib/clang/32023.883/lib/darwin/*")
file(COPY ${SDK_FILES} DESTINATION "${CMAKE_BINARY_DIR}/lib")

# 2. Configure compilation for our custom clean-room targets (which will overwrite the copied originals)

# metal_rt target
add_library(metal_rt STATIC src/metal_runtime.cpp)
target_include_directories(metal_rt PUBLIC include)
set_target_properties(metal_rt PROPERTIES POSITION_INDEPENDENT_CODE ON)

# air_rt target
add_library(air_rt STATIC src/air_rt.cpp)
target_include_directories(air_rt PUBLIC include)
set_target_properties(air_rt PROPERTIES POSITION_INDEPENDENT_CODE ON)

# resource_tracking_rt target
add_library(resource_tracking_rt STATIC src/resource_tracking_rt.cpp)
target_include_directories(resource_tracking_rt PUBLIC include)
set_target_properties(resource_tracking_rt PROPERTIES POSITION_INDEPENDENT_CODE ON)

# shader_logging_rt target
add_library(shader_logging_rt STATIC src/shader_logging_rt.cpp)
target_include_directories(shader_logging_rt PUBLIC include)
set_target_properties(shader_logging_rt PROPERTIES POSITION_INDEPENDENT_CODE ON)

# raytracing_rt target
add_library(raytracing_rt STATIC src/raytracing_rt.cpp)
target_include_directories(raytracing_rt PUBLIC include)
set_target_properties(raytracing_rt PROPERTIES POSITION_INDEPENDENT_CODE ON)

# post_mesh_dump_rt target
add_library(post_mesh_dump_rt STATIC src/post_mesh_dump_rt.cpp)
target_include_directories(post_mesh_dump_rt PUBLIC include)
set_target_properties(post_mesh_dump_rt PROPERTIES POSITION_INDEPENDENT_CODE ON)


# Custom post-build commands to overwrite copied originals with compiled replacements
add_custom_command(TARGET metal_rt POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_osx.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_ios.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_iossim.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_iosmac.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_tvos.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_tvossim.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_watchos.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_watchossim.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_xros.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_xrossim.a"
  COMMENT "Copying compiled libmetal_rt replacements over official binaries..."
)

add_custom_command(TARGET air_rt POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_osx.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_ios.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_iossim.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_iosmac.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_tvos.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_tvossim.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_watchos.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_watchossim.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_xros.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:air_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_xrossim.rtlib"
  COMMENT "Copying compiled libair_rt replacements over official binaries..."
)

add_custom_command(TARGET resource_tracking_rt POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_osx.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_ios.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_iossim.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_iosmac.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_tvos.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_tvossim.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_watchos.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_watchossim.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_xros.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resource_tracking_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_xrossim.rtlib"
  COMMENT "Copying compiled libresource_tracking_rt replacements over official binaries..."
)

add_custom_command(TARGET shader_logging_rt POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:shader_logging_rt> "${CMAKE_BINARY_DIR}/lib/MTLShaderLoggingRuntime.rtlib"
  COMMENT "Copying compiled MTLShaderLoggingRuntime replacement over official binary..."
)

add_custom_command(TARGET raytracing_rt POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:raytracing_rt> "${CMAKE_BINARY_DIR}/lib/MTLRaytracingRuntime.rtlib"
  COMMENT "Copying compiled MTLRaytracingRuntime replacement over official binary..."
)

add_custom_command(TARGET post_mesh_dump_rt POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_osx.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_ios.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_iossim.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_iosmac.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_tvos.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_tvossim.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_watchos.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_watchossim.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_xros.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:post_mesh_dump_rt> "${CMAKE_BINARY_DIR}/lib/libpost_mesh_dump_rt_xrossim.rtlib"
  COMMENT "Copying compiled libpost_mesh_dump_rt replacements over official binaries..."
)
