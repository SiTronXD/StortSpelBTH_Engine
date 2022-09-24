#pragma once 
#include "vk_mem_alloc.h"
#include <vulkan/vulkan.hpp>
#include "assimp/Importer.hpp" 
#include "Device.hpp"

// All vulkan handles required to create and delete mesh/texture resources
struct VulkanImportStructs{
    VmaAllocator*        vma; 
    vk::PhysicalDevice*  physicalDevice; 
    Device*              device; 
    vk::Queue*           transferQueue; 
    vk::CommandPool*     transferCommandPool; 
};

/*  Temporar wrapping struct for Texture related data, 
    will be refactored into Texture wrapper later. 
*/ 
struct ImageData {
        vk::ImageView   imageView;
        VmaAllocation   imageMemory;
        vk::Image       image;
        uint32_t        descriptorLocation;
};