add_llvm_component_library(LLVMAIRBitWriter
  AIRWriter/AIRBitcodeWriter.cpp
  AIRWriter/AIRPointerTypeAnalysis.cpp
  AIRWriter/AIRValueEnumerator.cpp
  AIRPrepare.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Target/AIR
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR

  LINK_COMPONENTS
  BitstreamWriter
  Core
  Support
  TargetParser
)
