#pragma once

#include <string>
#include "TempPCH.h"

class Instance;
class Device;

class VulkanDbg
{
private:
    static Instance* instance;
    static Device* device;

public:
    static void init(
        Instance& instance,
        Device& device
    );

    static void registerVkObjectDbgInfo(
        const std::string& name, 
        vk::ObjectType type, 
        uint64_t objectHandle);

    static void populateDebugMessengerCreateInfo(
        vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
};