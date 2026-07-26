set(LLVM_LINK_COMPONENTS
  Support
)

add_llvm_tool(llvm-metallib
  llvm-metallib.cpp

  DEPENDS
  intrinsics_gen
)
