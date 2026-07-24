set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_tool(metal-ld
  metal-ld.cpp
  )
