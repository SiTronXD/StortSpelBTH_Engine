#include "Utilities.h"
#define VMA_IMPLEMENTATION
#include "tracy/Tracy.hpp"
#include "vk_mem_alloc.h"
#include "Configurator.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "glm/fwd.hpp"
#include <vector>
#include <fstream>





namespace vengine_helper{

    //////__attribute__((unused))
[[maybe_unused]]  // To remove annoying warning... this function is used in another translation unit...
    std::vector<char> readFile(const std::string &filename) {
        #ifndef VENGINE_NO_PROFILING
            ZoneScoped; //:NOLINT
        #endif
        ///Open stream from given file as a binary
        std::ifstream file(filename, std::ios::binary  | std::ios::ate);
        ///std::ios::ate <-- stands for "at End", it means that we start at the end of the file rather than the beginning...
        /// this will tell us how big the file is, after we know that we will move the pointer to the beginning...

        ///Check if filestream successfully open...
        if(!file.is_open()){
            throw std::runtime_error("Failed to open a file");
        }
        
        
        unsigned long filesize = (unsigned long)file.tellg();
        std::vector<char> fileBuffer(filesize);
        file.seekg(0); /// move read position (seek) to the start of the file!
        file.read(fileBuffer.data(), filesize);
        file.close();

        return fileBuffer;
    }

    std::vector<char> readShaderFile(const std::string &filename) {
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
    void createBuffer(createBufferData &&bufferData) {
#ifndef VENGINE_NO_PROFILING
      ZoneScoped; //: NOLINT
#endif

      vk::BufferCreateInfo bufferInfo;
      bufferInfo.setSize(bufferData.bufferSize); /// Size of the buffer (size of
                                                 /// Vertex * nr of vertices)
      bufferInfo.setUsage(bufferData.bufferUsageFlags); /// The type of buffer
      bufferInfo.setSharingMode(
          vk::SharingMode::
              eExclusive); /// Define if the Buffer should be shared between
                           /// different Queues VK_SHARING_MODE_EXCLUSIVE   :
                           /// Only one queue can handle this Buffer
                           /// VK_SHARING_MODE_CONCURRENT  : Several Queues can
                           /// handle this Buffer

      VmaAllocationCreateInfo vmaAllocCreateInfo{};
      vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
      vmaAllocCreateInfo.flags = bufferData.bufferProperties;

      if (vmaCreateBuffer(*bufferData.vma, (VkBufferCreateInfo *)&bufferInfo,
                          &vmaAllocCreateInfo, (VkBuffer *)bufferData.buffer,
                          bufferData.bufferMemory,
                          bufferData.allocationInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to Allocate Image through VMA!");
      }
    }
    vk::CommandBuffer beginCommandBuffer(vk::Device device,
                                         vk::CommandPool commandPool) {
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
    void endAndSubmitCommandBuffer(vk::Device device,
                                   vk::CommandPool commandPool, vk::Queue queue,
                                   vk::CommandBuffer commandBuffer) {
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
        vk::CommandBuffer commandBuffer) {
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
    void copyBuffer(vk::Device device, vk::Queue transferQueue,
                           vk::CommandPool transferCommandPool,
                           vk::Buffer srcBuffer, vk::Buffer dstBuffer,
                           vk::DeviceSize bufferSize) {
#ifndef VENGINE_NO_PROFILING
      ZoneScoped; //: NOLINT
#endif

      /// Create Command Buffer
      vk::CommandBuffer transferCommandBuffer =
          beginCommandBuffer(device, transferCommandPool);

      /// Region of data to copy from and to
      vk::BufferCopy2 bufferCopyRegion;
      bufferCopyRegion.setSrcOffset(
          0); /// Copy everything from the start of the buffer
      bufferCopyRegion.setDstOffset(
          0); /// Copy everytinng to the beginning of the other buffer
      bufferCopyRegion.setSize(
          bufferSize); /// How much to copy, i.e. the whole buffer!

      /// Command to copy srcBuffer to  dstBuffer
      vk::CopyBufferInfo2 copyBufferInfo;
      copyBufferInfo.setDstBuffer(dstBuffer);
      copyBufferInfo.setSrcBuffer(srcBuffer);
      copyBufferInfo.setRegionCount(uint32_t(1));
      copyBufferInfo.setPRegions(&bufferCopyRegion);
      transferCommandBuffer.copyBuffer2(copyBufferInfo);

      endAndSubmitCommandBuffer(device, transferCommandPool, transferQueue,
                                transferCommandBuffer);
    }
    void copyImageBuffer(vk::Device device, vk::Queue transferQueue,
                                vk::CommandPool transferCommandPool,
                                vk::Buffer srcBuffer, vk::Image dstImage,
                                uint32_t width, uint32_t height) {
#ifndef VENGINE_NO_PROFILING
      ZoneScoped; //: NOLINT
#endif

      /// Create Command Buffer
      vk::CommandBuffer transferCommandBuffer =
          beginCommandBuffer(device, transferCommandPool);

      vk::BufferImageCopy2 imageRegion;
      imageRegion.setBufferOffset(0); /// offset into Data; start of file
      imageRegion.setBufferRowLength(
          uint32_t(0)); /// Row length of data, used to calculate data spacing
      imageRegion.setBufferImageHeight(
          uint32_t(0)); /// Image height to calculate data spacing; all data is
                        /// tightly packed, no space between pixels
      imageRegion.imageSubresource.setAspectMask(
          vk::ImageAspectFlagBits::eColor); /// Which aspect of the Image to
                                            /// Copy
      imageRegion.imageSubresource.setMipLevel(
          uint32_t(0)); /// MipMap level to copy, we dont use it so set it to 0.
      imageRegion.imageSubresource.setBaseArrayLayer(
          uint32_t(0)); /// Starting array layer, we dont use array layers so it
                        /// will be 0
      imageRegion.imageSubresource.setLayerCount(
          uint32_t(1)); /// Number of Layers to copy starting at baseArrayLayer,
                        /// we dont have several layers... so 1!
      imageRegion.setImageOffset(vk::Offset3D(
          0, 0,
          0)); /// Offset Into Image, (as iooised to raw data in bufferOffset)
               /// Start from the Origin and copy everything from there
      imageRegion.setImageExtent(vk::Extent3D(
          width, height, 1)); /// Size of region to copy as (x,y,z) values

      /// Copy buffer to given image
      vk::CopyBufferToImageInfo2 copyBufferToImageInfo;
      copyBufferToImageInfo.setDstImage(dstImage);
      copyBufferToImageInfo.setDstImageLayout(
          vk::ImageLayout::eTransferDstOptimal); /// Images requires the
                                                 /// Transfer_DST_optimal, to be
                                                 /// in an optimal state to
                                                 /// transfer data to it
      copyBufferToImageInfo.setSrcBuffer(srcBuffer);
      copyBufferToImageInfo.setRegionCount(
          uint32_t(1)); /// We only use one, but we coult use several
      copyBufferToImageInfo.setPRegions(&imageRegion); /// The region we defined
      transferCommandBuffer.copyBufferToImage2(copyBufferToImageInfo);

      endAndSubmitCommandBuffer(device, transferCommandPool, transferQueue,
                                transferCommandBuffer);
    }
    void transitionImageLayout(vk::Device device, vk::Queue queue,
                               vk::CommandPool commandPool, vk::Image image,
                               vk::ImageLayout oldLayout,
                               vk::ImageLayout newLayout) {
#ifndef VENGINE_NO_PROFILING
      ZoneScoped; //: NOLINT
#endif
                  /// Create Buffer
      vk::CommandBuffer commandBuffer = beginCommandBuffer(device, commandPool);

      /// MemoryBarrier allows us to transition between two layouts!
      vk::ImageMemoryBarrier2 imageMemoryBarrier;
      imageMemoryBarrier.setOldLayout(oldLayout); /// Layout to transition from
      imageMemoryBarrier.setNewLayout(newLayout); /// Layout to transition from
      imageMemoryBarrier.setSrcQueueFamilyIndex(
          VK_QUEUE_FAMILY_IGNORED); /// Queue Family to transition from
      imageMemoryBarrier.setDstQueueFamilyIndex(
          VK_QUEUE_FAMILY_IGNORED); /// Queue Family to transition to
      imageMemoryBarrier.setImage(
          image); /// Image being accessed and modified as part of barrier
      imageMemoryBarrier.subresourceRange.setAspectMask(
          vk::ImageAspectFlagBits::eColor); /// Aspect of the image being
                                            /// altered
      imageMemoryBarrier.subresourceRange.setBaseMipLevel(
          uint32_t(0)); /// First mip level to start alterations on
      imageMemoryBarrier.subresourceRange.setLevelCount(uint32_t(
          1)); /// Number of mipmap levels to alter starting from baseMipLevel
      imageMemoryBarrier.subresourceRange.setBaseArrayLayer(
          uint32_t(0)); /// First layers to start alterations on
      imageMemoryBarrier.subresourceRange.setLayerCount(uint32_t(
          1)); /// Number of layers to alter starting from baseArrayLayer

      /// If transitioning from 'new image' to 'image ready' to 'recieve data'
      if (oldLayout == vk::ImageLayout::eUndefined &&
          newLayout == vk::ImageLayout::eTransferDstOptimal) {
        /// Defining that before ANY TransferWrite happens (no matter what state
        /// it is in),
        //// a transition between oldLayout to newLayout MUST happen!
        //// i.e. A layout transition MUST happen before we attempt to do a
        ///Transfer Write (VK_ACCESS_TRANSFER_WRITE_BIT)
        imageMemoryBarrier.setSrcAccessMask(
            vk::AccessFlagBits2::eNone); /// srcState 0 means any State TODO::
                                         /// Does none work here?
        imageMemoryBarrier.setDstAccessMask(
            vk::AccessFlagBits2::eTransferWrite); /// The dstState cant execute
                                                  /// before srcState finishes!
        /// transition must happens after srcAccessMask
        /// transition must happens before dstAccessMask

        imageMemoryBarrier.setSrcStageMask(
            vk::PipelineStageFlagBits2::
                eTopOfPipe); /// Transition must happen  after srcStage: Any
                             /// stage from (at?) the top of the pipeline
        imageMemoryBarrier.setDstStageMask(
            vk::PipelineStageFlagBits2::
                eTransfer); /// Transition must happen before dstStage: The
                            /// TransferStage of the Pipeline (where it tried to
                            /// do a TransferWrite!)

      } /// If transitioning from Transfer Destination to Shader Readable
      else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        imageMemoryBarrier.setSrcAccessMask(
            vk::AccessFlagBits2::
                eTransferWrite); /// The srcState must be finished with writing
                                 /// before layout transition can happen!
        imageMemoryBarrier.setDstAccessMask(
            vk::AccessFlagBits2::eShaderRead); /// The dstState transition
                                               /// must've happen before Shader
                                               /// will read the data.

        imageMemoryBarrier.setSrcStageMask(
            vk::PipelineStageFlagBits2::
                eTransfer); /// Transition must happen  after srcStage:
                            /// Transfered the data to the ... (??)

        imageMemoryBarrier.setDstStageMask(
            vk::PipelineStageFlagBits2::
                eFragmentShader); /// Transition must happen before dstStage:
                                  /// Transfer the data to the Fragment Shader
                                  /// Stage
      }

      /// Generic Pipeline Barrier is used for all Barriers, thus some arguments
      /// will not be used
      vk::DependencyInfo dependencyInfo;
      dependencyInfo.setDependencyFlags(
          vk::DependencyFlagBits::eByRegion); // TODO :: is this the same as
                                              // setting flagBits to 0? ...
      dependencyInfo.setBufferMemoryBarrierCount(uint32_t(0));
      dependencyInfo.setPBufferMemoryBarriers(nullptr);
      dependencyInfo.setImageMemoryBarrierCount(uint32_t(1));
      dependencyInfo.setImageMemoryBarriers(imageMemoryBarrier);
      dependencyInfo.setMemoryBarrierCount(uint32_t(0));
      dependencyInfo.setPMemoryBarriers(nullptr);

      commandBuffer.pipelineBarrier2(dependencyInfo);

      endAndSubmitCommandBuffer(device, commandPool, queue, commandBuffer);
    }
    void insertImageMemoryBarrier(createImageBarrierData &&imageBarrierData) {
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