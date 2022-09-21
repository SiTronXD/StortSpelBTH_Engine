#pragma once
///This file will contain generic functions and structures that will be used across the whole program...
#include <array>
#include <vulkan/vulkan.hpp>
#include "glm/glm.hpp"


//Forward declarattions
struct VmaAllocator_T;
struct VmaAllocation_T;
struct VmaAllocationInfo;
using VmaAllocation = struct VmaAllocation_T*;
using VmaAllocator = struct VmaAllocator_T*;
using VmaAllocationCreateFlags = VkFlags;

 

const int MAX_OBJECTS = 250; /// Maximum number of objects to allocate memory for

/// Defines all the Device Extensions to be used
constexpr std::array<const char *, 2> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME,  

        //"VK_KHR_portability_subset"                   /// Used for profiling VP_LUNARG_desktop_portability_2021
        //VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,      /// Deprecated, this extension is now part of vk_1.3
        //VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,      /// Deprecated, this extension is now part of vk_1.3
        //VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,    /// Deprecated, this extension is now part of vk_1.2
};

/// Defines extra Instance Extensions to be used (debug and SDL extensions are added in createInstance...)
constexpr std::array<const char *, 1> extraInstanceExtensions = {
    VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME
    
};

struct Vertex 
{
    glm::vec3 pos;      /// Vertex Position (x,y,z)
    glm::vec3 col;      /// Vertex Color    (r,g,b)
    glm::vec2 tex;      /// texture coords  (u,v)
};

struct CreateImageBarrierData
{
    vk::CommandBuffer cmdBuffer; 
    vk::Image image; 
    vk::AccessFlags2 srcAccessMask;    
    vk::AccessFlags2 dstAccessMask; 
    vk::ImageLayout oldLayout; 
    vk::ImageLayout newLayout;
    vk::PipelineStageFlags2 srcStageMask ;
    vk::PipelineStageFlags2 dstStageMask ;
    vk::ImageSubresourceRange imageSubresourceRange;
};

namespace vengine_helper
{

    ////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is used in another translation unit...
    std::vector<char> readFile(const std::string &filename) ;

    
    ////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is used in another translation unit...
    std::vector<char> readShaderFile(const std::string &filename);

    ////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is                        

     uint32_t findMemoryTypeIndex( vk::PhysicalDevice physicalDevice, uint32_t allowedTypes,vk::MemoryPropertyFlags properties);

    vk::CommandBuffer beginCommandBuffer(vk::Device device,
                                         vk::CommandPool commandPool);

    void endAndSubmitCommandBuffer(vk::Device device,
                                   vk::CommandPool commandPool, vk::Queue queue,
                                   vk::CommandBuffer commandBuffer);

    void endAndSubmitCommandBufferWithFences(
        vk::Device device, vk::CommandPool commandPool, vk::Queue queue,
        vk::CommandBuffer commandBuffer);

    void insertImageMemoryBarrier(
        CreateImageBarrierData &&imageBarrierData);
}
