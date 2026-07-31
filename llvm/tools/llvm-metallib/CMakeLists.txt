set(LLVM_LINK_COMPONENTS
  BinaryFormat
  BitReader
  BitWriter
  Core
  IRReader
  Linker
  Object
  Support
  )

add_llvm_tool(llvm-metallib
  llvm-metallib.cpp

  DEPENDS
  intrinsics_gen
  )
