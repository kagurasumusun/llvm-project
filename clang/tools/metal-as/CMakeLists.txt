set(LLVM_LINK_COMPONENTS
  BitReader
  BitWriter
  Core
  Support
  )

add_clang_tool(metal-as
  metal-as.cpp
  )
