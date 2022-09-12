#include "VulkanDbg.h"

#include "Device.h"

vk::Instance VulkanDbg::instance = {};
Device* VulkanDbg::device = nullptr;

void VulkanDbg::init(const vk::Instance& instance, Device& device)
{
    VulkanDbg::instance = instance;
    VulkanDbg::device = &device;
}

void VulkanDbg::registerVkObjectDbgInfo(
	const std::string& name, 
	vk::ObjectType type, 
	uint64_t objectHandle)
{
#ifdef DEBUG

    vk::DebugUtilsObjectNameInfoEXT objInfo;
    objInfo.setPObjectName(name.c_str());
    objInfo.setObjectType(type);
    objInfo.setObjectHandle(objectHandle); //NOLINT: reinterpret cast is ok here...
    objInfo.setPNext(nullptr);

    auto pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(VulkanDbg::instance, "vkSetDebugUtilsObjectNameEXT"); //(!!)

    auto temp = VkDebugUtilsObjectNameInfoEXT(objInfo);
    pfnSetDebugUtilsObjectNameEXT(VulkanDbg::device->getVkDevice(), &temp); //(!!)

#endif  
}
