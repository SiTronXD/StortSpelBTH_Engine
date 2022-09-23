#include "tracyHelper.hpp" 


#ifndef VENGINE_NO_PROFILING    
#include "tracy/Tracy.hpp"
#include "../graphics/VulkanRenderer.hpp"

VulkanRenderer* ref;


void TracyHelper::registerTracyParameterFunctions() 
{
  TracyParameterSetup(0, "Use Screenshot", true, 0);

}

void TracyHelper::setVulkanRenderReference(VulkanRenderer *_ref) { 
    ref = _ref;
}


VulkanRenderer* TracyHelper::getVulkanRenderReference() { 
    return ref;
}
#endif // VENGINE_NO_PROFILING