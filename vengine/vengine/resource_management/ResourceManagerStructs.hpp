#pragma once 
#include <vulkan/vulkan.hpp>
#include "../graphics/vulkan/PhysicalDevice.hpp"
#include "../graphics/vulkan/Device.hpp"
#include "../graphics/vulkan/VmaUsage.hpp"

// All vulkan handles required to create and delete mesh/texture resources
struct VulkanImportStructs
{
    VmaAllocator*        vma; 
    PhysicalDevice*  physicalDevice; 
    Device*              device; 
    vk::Queue*           transferQueue; 
    vk::CommandPool*     transferCommandPool; 
};