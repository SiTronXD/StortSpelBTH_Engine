#pragma once

#include <string>
#include "TempPCH.h"

class Device;

class VulkanDbg
{
private:
    static vk::Instance instance;
    static Device* device;

public:
    static void init(
        const vk::Instance& instance,
        Device& device
    );

    static void registerVkObjectDbgInfo(
        const std::string& name, 
        vk::ObjectType type, 
        uint64_t objectHandle);

};