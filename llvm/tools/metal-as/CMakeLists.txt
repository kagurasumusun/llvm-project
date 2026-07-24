set(LLVM_LINK_COMPONENTS
  BitReader
  BitWriter
  Core
  Support
  )

add_llvm_tool(metal-as
  metal-as.cpp
  )
