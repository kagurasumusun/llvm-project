add_llvm_component_library(LLVMMetalInfo
  MetalTargetInfo.cpp

  LINK_COMPONENTS
  Support
  TargetParser

  ADD_TO_COMPONENT
  Metal
)
