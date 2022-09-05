#include "tracyHelper.h" 


#ifndef VENGINE_NO_PROFILING    
#include "tracy/Tracy.hpp"
#include "VulkanRenderer.h"

VulkanRenderer* ref;

void TracyHelper::toggleTracyThumbnail(uint32_t idx, int32_t val) //:NOLINT: Cant Change methods signature since Tracy depends on it
{
  VulkanRenderer *renderer = getVulkanRenderReference();
  //renderer->toggleTracyThumbnail(static_cast<bool>(val)); // DEPRECATED
}

void TracyHelper::registerTracyParameterFunctions() 
{
  TracyParameterSetup(0, "Use Screenshot", true, 0);
  TracyParameterRegister(toggleTracyThumbnail);
}

void TracyHelper::setVulkanRenderReference(VulkanRenderer *_ref) { 
    ref = _ref;
}


VulkanRenderer* TracyHelper::getVulkanRenderReference() { 
    return ref;
}
#endif // VENGINE_NO_PROFILING