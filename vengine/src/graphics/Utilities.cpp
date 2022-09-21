#include "Utilities.hpp"
#define VMA_IMPLEMENTATION
#include "tracy/Tracy.hpp"
#include "vk_mem_alloc.h"
#include "../loaders/Configurator.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "glm/fwd.hpp"
#include <vector>
#include <fstream>

namespace vengine_helper
{
    //////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is used in another translation unit...
    std::vector<char> readFile(const std::string &filename) 
    {
        #ifndef VENGINE_NO_PROFILING
            ZoneScoped; //:NOLINT
        #endif
        ///Open stream from given file as a binary
        std::ifstream file(filename, std::ios::binary  | std::ios::ate);
        ///std::ios::ate <-- stands for "at End", it means that we start at the end of the file rather than the beginning...
        /// this will tell us how big the file is, after we know that we will move the pointer to the beginning...

        ///Check if filestream successfully open...
        if(!file.is_open())
        {
            throw std::runtime_error("Failed to open a file: " + filename);
        }
        
        
        unsigned long filesize = (unsigned long)file.tellg();
        std::vector<char> fileBuffer(filesize);
        file.seekg(0); /// move read position (seek) to the start of the file!
        file.read(fileBuffer.data(), filesize);
        file.close();

        return fileBuffer;
    }

    std::vector<char> readShaderFile(const std::string &filename) 
    {
        #ifndef VENGINE_NO_PROFILING
            ZoneScoped; //:NOLINT
        #endif
        using namespace vengine_helper::config;
        return readFile(DEF<std::string>(P_SHADERS)+filename);
    }
    ////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is                            
    uint32_t findMemoryTypeIndex(
        vk::PhysicalDevice physicalDevice,
        uint32_t allowedTypes,
        vk::MemoryPropertyFlags properties) 
    {
    #ifndef VENGINE_NO_PROFILING
        ZoneScoped; //: NOLINT
    #endif
      // Get properties of Physical Device Memory
      vk::PhysicalDeviceMemoryProperties2 memProperties;
      memProperties = physicalDevice.getMemoryProperties2();

      for (uint32_t i = 0; i < memProperties.memoryProperties.memoryTypeCount;
           i++) {
        /// Bitwise operation to determine if type is allowed!
        if (((allowedTypes & (1U << i)) !=
             0U) /// Index of memory type must match corresponding bit in
                 /// allowedTypes
            && (memProperties.memoryProperties.memoryTypes[i].propertyFlags &
                properties) ==
                   properties) /// Check if Desired Proprety Bit flags are part
                               /// of memory type's property flags
        {
          /// Valid memory type to return!
          return i;
        }
      }
      throw std::runtime_error(
          "Failed to find any of the allowed memory types");
    }
    
    vk::CommandBuffer beginCommandBuffer(vk::Device device,
                                         vk::CommandPool commandPool) 
    {
#ifndef VENGINE_NO_PROFILING
      ZoneScoped; //: NOLINT
#endif
                  /// Command Buffer to hold transfer Commands
      vk::CommandBuffer commandBuffer;

      /// Commaand Buffer Details
      vk::CommandBufferAllocateInfo allocInfo;
      allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
      allocInfo.setCommandPool(commandPool);
      allocInfo.setCommandBufferCount(uint32_t(1));

      /// Allocate CommandBuffer from pool
      commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

      /// Information to begin the command buffer Record
      vk::CommandBufferBeginInfo beginInfo;
      beginInfo.setFlags(
          vk::CommandBufferUsageFlagBits::
              eOneTimeSubmit); /// This command buffer is only used once, so set
                               /// it up for one submit

      /// Begin recording Transfer Commands
      commandBuffer.begin(beginInfo);

      return commandBuffer;
    }

    void endAndSubmitCommandBuffer(
        vk::Device device,
        vk::CommandPool commandPool, 
        vk::Queue queue,
        vk::CommandBuffer commandBuffer) 
    {
#ifndef VENGINE_NO_PROFILING
      ZoneScoped; //: NOLINT
#endif

      commandBuffer.end();

      /// Queue Submission information
      vk::SubmitInfo2 submitInfo;
      vk::CommandBufferSubmitInfo commandbufferSubmitInfo;
      commandbufferSubmitInfo.setCommandBuffer(commandBuffer);
      submitInfo.setCommandBufferInfoCount(uint32_t(1));
      submitInfo.setPCommandBufferInfos(&commandbufferSubmitInfo);

      /// Submit Transfer Commands to transfer Queue and wait for it to finish
      queue.submit2(submitInfo);

      queue.waitIdle(); // TODO: this is bad, use proper synchronization
                        // instead!

      /// Free temporary Command Buffer back to pool
      device.freeCommandBuffers(commandPool, uint32_t(1), &commandBuffer);
    }

    void endAndSubmitCommandBufferWithFences(
        vk::Device device, vk::CommandPool commandPool, vk::Queue queue,
        vk::CommandBuffer commandBuffer) 
    {
#ifndef VENGINE_NO_PROFILING
      ZoneScoped; //: NOLINT
#endif
      uint64_t const fence_timeout_ms = 100000000000;
      commandBuffer.end();

      /// Queue Submission information
      vk::SubmitInfo2 submitInfo;
      vk::CommandBufferSubmitInfo commandbufferSubmitInfo;
      commandbufferSubmitInfo.setCommandBuffer(commandBuffer);
      submitInfo.setCommandBufferInfoCount(uint32_t(1));
      submitInfo.setPCommandBufferInfos(&commandbufferSubmitInfo);

      vk::FenceCreateInfo fenceInfo;
      fenceInfo.setFlags(
          vk::FenceCreateFlagBits::eSignaled); // TODO:: this must be right,
                                               // enum has only one value...
      vk::Fence fence = device.createFence(fenceInfo);

      /// Submit Transfer Commands to transfer Queue and wait for it to finish
      queue.submit2(submitInfo, fence);

      if (device.waitForFences(fence, VK_TRUE, fence_timeout_ms) !=
          vk::Result::eSuccess) {
        throw std::runtime_error("Failed at waiting for a Fence!");
      }
      device.destroyFence(fence);

      /// Free temporary Command Buffer back to pool
      device.freeCommandBuffers(commandPool, commandBuffer);
    }

    void insertImageMemoryBarrier(
        CreateImageBarrierData &&imageBarrierData) 
    {
#ifndef VENGINE_NO_PROFILING
      ZoneScoped; //: NOLINT
#endif
                  /// MemoryBarrier allows us to transition between two layouts!
      vk::ImageMemoryBarrier2 imageMemoryBarrier;
      imageMemoryBarrier.setOldLayout(
          imageBarrierData.oldLayout); /// Layout to transition from
      imageMemoryBarrier.setNewLayout(
          imageBarrierData.newLayout); /// Layout to transition from
      imageMemoryBarrier.setSrcQueueFamilyIndex(
          VK_QUEUE_FAMILY_IGNORED); /// Queue Family to transition from
      imageMemoryBarrier.setDstQueueFamilyIndex(
          VK_QUEUE_FAMILY_IGNORED); /// Queue Family to transition from
      imageMemoryBarrier.setImage(
          imageBarrierData
              .image); /// Image being accessed and modified as part of barrier

      imageMemoryBarrier.setSubresourceRange(
          imageBarrierData.imageSubresourceRange);
      imageMemoryBarrier.setSrcAccessMask(imageBarrierData.srcAccessMask);
      imageMemoryBarrier.setDstAccessMask(imageBarrierData.dstAccessMask);

      imageMemoryBarrier.setSrcStageMask(imageBarrierData.srcStageMask);
      imageMemoryBarrier.setDstStageMask(imageBarrierData.dstStageMask);

      /// Generic Pipeline Barrier is used for all Barriers, thus some arguments
      /// will not be used
      vk::DependencyInfo dependencyInfo;
      dependencyInfo.setDependencyFlags(
          vk::DependencyFlags()); // TODO :: is this the same as setting
                                  // flagBits to 0? ...
      dependencyInfo.setBufferMemoryBarrierCount(uint32_t(0));
      dependencyInfo.setPBufferMemoryBarriers(nullptr);
      dependencyInfo.setImageMemoryBarrierCount(uint32_t(1));
      dependencyInfo.setImageMemoryBarriers(
          imageMemoryBarrier); /// Image Memory Barrier Data
      dependencyInfo.setMemoryBarrierCount(uint32_t(0));
      dependencyInfo.setPMemoryBarriers(nullptr); /// Buffer Memory Barrier Data

      imageBarrierData.cmdBuffer.pipelineBarrier2(dependencyInfo);
    }
} // namespace vengine_helper