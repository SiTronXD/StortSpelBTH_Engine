#include "pch.h"
#include "VulkanDbg.hpp"
#include "VulkanValidation.hpp"
#include "VulkanInstance.hpp"
#include "Device.hpp"

VulkanInstance* VulkanDbg::instance = nullptr;
Device* VulkanDbg::device = nullptr;

void VulkanDbg::init(VulkanInstance& instance, Device& device)
{
    VulkanDbg::instance = &instance;
    VulkanDbg::device = &device;
}

void VulkanDbg::registerVkObjectDbgInfo(
	const std::string& name, 
	vk::ObjectType type, 
	uint64_t objectHandle)
{
    if (isValidationLayersEnabled())
    {
        vk::DebugUtilsObjectNameInfoEXT objInfo;
        objInfo.setPObjectName(name.c_str());
        objInfo.setObjectType(type);
        objInfo.setObjectHandle(objectHandle); //NOLINT: reinterpret cast is ok here...
        objInfo.setPNext(nullptr);

        auto pfnSetDebugUtilsObjectNameEXT =
            (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
                VulkanDbg::instance->getVkInstance(),
                "vkSetDebugUtilsObjectNameEXT"
            ); //(!!)

        auto temp = VkDebugUtilsObjectNameInfoEXT(objInfo);
        pfnSetDebugUtilsObjectNameEXT(VulkanDbg::device->getVkDevice(), &temp); //(!!)

    }
}

void VulkanDbg::populateDebugMessengerCreateInfo(
    vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    createInfo.messageSeverity
        = /*vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
        |*/ vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
        | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    createInfo.messageType
        = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
        | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
        | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    createInfo.setPfnUserCallback(debugCallback);
    createInfo.pUserData = nullptr;
}
