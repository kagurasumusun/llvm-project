set(LLVM_LINK_COMPONENTS
  Support
  Core
  BitWriter
  BitReader
)

add_llvm_tool(llvm-metallib
  llvm-metallib.cpp
)
