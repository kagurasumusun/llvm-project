#===-- runtimes/metal_rt/CMakeLists.txt - Metal Runtime Build Setup -----===#
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# Builds native static/runtime libraries for Metal substitute runtime:
#   - libmetal_rt.a
#   - libair_rt.rtlib
#   - libresource_tracking_rt.rtlib
#   - MTLShaderLoggingRuntime.rtlib
#
#===----------------------------------------------------------------------===#

add_library(metal_rt STATIC
  src/metal_runtime.cpp
)

target_include_directories(metal_rt PUBLIC include)

set_target_properties(metal_rt PROPERTIES
  OUTPUT_NAME "metal_rt"
  POSITION_INDEPENDENT_CODE ON
)

# Custom rule to create alias runtime libraries for compatibility
add_custom_command(TARGET metal_rt POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libmetal_rt_osx.a"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libair_rt_osx.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/libresource_tracking_rt_osx.rtlib"
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:metal_rt> "${CMAKE_BINARY_DIR}/lib/MTLShaderLoggingRuntime.rtlib"
  COMMENT "Copying Metal Runtime alternative libraries to build/lib/"
)
