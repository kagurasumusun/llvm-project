add_llvm_component_library(LLVMMetalDesc
  MCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  MetalInfo
  Support

  ADD_TO_COMPONENT
  Metal
)
