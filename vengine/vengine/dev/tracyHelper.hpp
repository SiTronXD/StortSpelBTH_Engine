#pragma once 
 #include "op_overload.hpp"
#include <cstdint>
#ifndef VENGINE_NO_PROFILING    
class VulkanRenderer;

namespace TracyHelper{

    void setVulkanRenderReference(VulkanRenderer *_ref);        
    VulkanRenderer *getVulkanRenderReference();
    void registerTracyParameterFunctions();
}
#endif