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

const int MAX_FRAME_DRAWS = 2; /// Should be Atleast one less than we have in our SwapChain!
//const int MAX_FRAME_DRAWS = 3; /// Should be Atleast one less than we have in our SwapChain!

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

/// Indices (locations) of Queue Families (if they exists at all...)
struct QueueFamilyIndices {
    /// Note! Both the GraphicsFamily and PresentationFamily queue might be the same QueueFamily!
    /// - Presentation is actually a Feature of a Queue Family, rather than being a seperate Queue Family Type...!
    int32_t graphicsFamily = -1 ;       /// Location of the Graphics Queue Family!
    int32_t presentationFamily = -1;

    ///Check if Queue Families are valid... (Invalid Families will have -1 as value...)
    [[nodiscard]] bool isValid() const{
        return graphicsFamily >= 0 && presentationFamily >= 0;
    }
};

/// Defines what kind of surface we can create with our given surface
struct SwapChainDetails {
    vk::SurfaceCapabilities2KHR surfaceCapabilities;       /// Surface Properties, Image size/extent
    /*!Describes Information about what the surface can handle:
     * - minImageCount/maxImageCount         : defines the min/max amount of image our surface can handle
     * - currentExtent/minExtent/maxExtent   : defines the size of a image
     * - supportedTransforms/currentTransform: ...
     * */
     std::vector<vk::SurfaceFormat2KHR> Format;          /// Surface Image Formats, RGBA and colorspace (??)
     /*!Describes the format for the Color Space and how the data is represented...
      * */
      std::vector<vk::PresentModeKHR> presentationMode; /// The presentation mode that our Swapchain will use.
    /*!How images should be presented to screen...
     * */

    [[nodiscard]] 
    bool isValid() const{
        return !Format.empty() && !presentationMode.empty();
    }
};

struct SwapChainImage {
    vk::Image     image;
    vk::ImageView imageView;

};

struct createImageData{
    uint32_t width;
    uint32_t height; 
    vk::Format format; 
    vk::ImageTiling tiling; 
    vk::ImageUsageFlags useFlags; 
    vk::MemoryPropertyFlags property; 
    //vk::DeviceMemory *imageMemory;
    VmaAllocation *imageMemory;
};

struct createBufferData{
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::DeviceSize bufferSize;
    vk::BufferUsageFlags bufferUsageFlags;
    // vk::MemoryPropertyFlags bufferProperties;
    VmaAllocationCreateFlags bufferProperties; //TODO: Replace this name... (!!)
    vk::Buffer* buffer;
    //vk::DeviceMemory* bufferMemory;
    VmaAllocation *bufferMemory;
    VmaAllocationInfo *allocationInfo = nullptr;
    VmaAllocator* vma;
};

struct createImageBarrierData{
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

namespace vengine_helper{

    ////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is used in another translation unit...
    std::vector<char> readFile(const std::string &filename) ;

    
    ////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is used in another translation unit...
    std::vector<char> readShaderFile(const std::string &filename);

    ////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is                        

     uint32_t findMemoryTypeIndex( vk::PhysicalDevice physicalDevice, uint32_t allowedTypes,vk::MemoryPropertyFlags properties);

    void createBuffer(createBufferData &&bufferData);

    vk::CommandBuffer beginCommandBuffer(vk::Device device,
                                         vk::CommandPool commandPool);

    void endAndSubmitCommandBuffer(vk::Device device,
                                   vk::CommandPool commandPool, vk::Queue queue,
                                   vk::CommandBuffer commandBuffer);

    void endAndSubmitCommandBufferWithFences(
        vk::Device device, vk::CommandPool commandPool, vk::Queue queue,
        vk::CommandBuffer commandBuffer);

    void copyBuffer(vk::Device device, vk::Queue transferQueue,
                           vk::CommandPool transferCommandPool,
                           vk::Buffer srcBuffer, vk::Buffer dstBuffer,
                           vk::DeviceSize bufferSize);

    void copyImageBuffer(vk::Device device, vk::Queue transferQueue,
                                vk::CommandPool transferCommandPool,
                                vk::Buffer srcBuffer, vk::Image dstImage,
                                uint32_t width, uint32_t height);

    void transitionImageLayout(vk::Device device, vk::Queue queue,
                               vk::CommandPool commandPool, vk::Image image,
                               vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout);

    void insertImageMemoryBarrier(createImageBarrierData &&imageBarrierData);
}
