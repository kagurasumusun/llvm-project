add_llvm_component_library(LLVMMetalCodeGen
  MetalTargetMachine.cpp
  MetalSubtarget.cpp
  MetalTargetObjectFile.cpp
  MetalAsmPrinter.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  Core
  MC
  MetalDesc
  MetalInfo
  Support
  Target

  ADD_TO_COMPONENT
  Metal
)
