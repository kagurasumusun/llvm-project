set(LLVM_LINK_COMPONENTS
  BitReader
  BitWriter
  Core
  Support
  )

add_clang_tool(metal-as
  metal-as.cpp
  )

clang_target_link_libraries(metal-as
  PRIVATE
  clangBasic
  )
