set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_tool(metal-ld
  metal-ld.cpp
  )

clang_target_link_libraries(metal-ld
  PRIVATE
  clangBasic
  )
