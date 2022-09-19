#pragma once

#include <string>
#include "../TempPCH.hpp"

class VulkanInstance;
class Device;

class VulkanDbg
{
private:
    static VulkanInstance* instance;
    static Device* device;

public:
    static void init(
        VulkanInstance& instance,
        Device& device
    );

    static void registerVkObjectDbgInfo(
        const std::string& name, 
        vk::ObjectType type, 
        uint64_t objectHandle);

    static void populateDebugMessengerCreateInfo(
        vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
};