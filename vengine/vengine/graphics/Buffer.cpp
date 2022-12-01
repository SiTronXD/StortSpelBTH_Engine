#include "pch.h"
#include "Buffer.hpp"

#include "vulkan/Device.hpp"
#include "vulkan/CommandBuffer.hpp"
#include "../dev/Log.hpp"

void Buffer::map(void*& outputWriteData, const uint32_t& bufferIndex)
{
    vmaMapMemory(
        *this->vma,
        this->bufferMemories[bufferIndex],
        &outputWriteData
    );
}

void Buffer::unmap(const uint32_t& bufferIndex)
{
    vmaUnmapMemory(
        *this->vma,
        this->bufferMemories[bufferIndex]
    );
}

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
}

void Buffer::init(
    Device& device,
    VmaAllocator& vma,
    const size_t& bufferSize)
{
    this->device = &device;
    this->vma = &vma;
    this->bufferSize = bufferSize;
}

void Buffer::update(void* copyData, const uint32_t& currentFrame)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    void* data = nullptr;

    this->cpuUpdateBuffer(
        *this->vma, 
        this->bufferMemories[currentFrame],
        this->getBufferSize(),
        copyData
    );
}

void Buffer::cleanup()
{
    for (size_t i = 0; i < this->buffers.size(); i++)
    {
        this->device->getVkDevice().destroyBuffer(this->buffers[i]);
        vmaFreeMemory(*this->vma, this->bufferMemories[i]);
    }
}

void Buffer::createBuffer(BufferCreateData&& bufferData)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif

    vk::BufferCreateInfo bufferInfo;
    bufferInfo.setSize(bufferData.bufferSize);
    bufferInfo.setUsage(bufferData.bufferUsageFlags); // The type of buffer
    bufferInfo.setSharingMode(vk::SharingMode:: eExclusive);

    VmaAllocationCreateInfo vmaAllocCreateInfo{};
    vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    vmaAllocCreateInfo.flags = bufferData.bufferAllocationFlags;

    if (vmaCreateBuffer(
        *bufferData.vma, 
        (VkBufferCreateInfo*) &bufferInfo,
        &vmaAllocCreateInfo, 
        (VkBuffer*) bufferData.buffer,
        bufferData.bufferMemory,
        bufferData.allocationInfo) != VK_SUCCESS) 
    {
        Log::error("Failed to allocate buffer through VMA.");
    }
}

void Buffer::copyBuffer(
    Device& device,
    const vk::Queue& transferQueue,
    const vk::CommandPool& transferCommandPool,
    const vk::Buffer& srcBuffer,
    const vk::Buffer& dstBuffer,
    const vk::DeviceSize& bufferSize)
{
    Buffer::copyBuffer(
        device.getVkDevice(),
        transferQueue,
        transferCommandPool,
        srcBuffer,
        dstBuffer,
        bufferSize
    );
}

void Buffer::copyBuffer(
    vk::Device& device,
    const vk::Queue& transferQueue,
    const vk::CommandPool& transferCommandPool,
    const vk::Buffer& srcBuffer,
    const vk::Buffer& dstBuffer,
    const vk::DeviceSize& bufferSize)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif

    // Create Command Buffer
    vk::CommandBuffer transferCommandBuffer =
        CommandBuffer::beginCommandBuffer(
            device,
            transferCommandPool
        );

    // Region of data to copy from and to
    vk::BufferCopy2 bufferCopyRegion;
    bufferCopyRegion.setSrcOffset(
        0); // Copy everything from the start of the buffer
    bufferCopyRegion.setDstOffset(
        0); // Copy everytinng to the beginning of the other buffer
    bufferCopyRegion.setSize(
        bufferSize); // How much to copy, i.e. the whole buffer!

    // Command to copy srcBuffer to  dstBuffer
    vk::CopyBufferInfo2 copyBufferInfo;
    copyBufferInfo.setDstBuffer(dstBuffer);
    copyBufferInfo.setSrcBuffer(srcBuffer);
    copyBufferInfo.setRegionCount(uint32_t(1));
    copyBufferInfo.setPRegions(&bufferCopyRegion);
    transferCommandBuffer.copyBuffer2(copyBufferInfo);

    CommandBuffer::endAndSubmitCommandBuffer(
        device,
        transferCommandPool,
        transferQueue,
        transferCommandBuffer
    );
}

void Buffer::copyBufferToImage(
    Device& device,
    const vk::Queue& transferQueue,
    const vk::CommandPool& transferCommandPool,
    const vk::Buffer& srcBuffer, 
    const vk::Image& dstImage,
    const uint32_t& width, 
    const uint32_t& height)
{
    Buffer::copyBufferToImage(
        device.getVkDevice(),
        transferQueue,
        transferCommandPool,
        srcBuffer,
        dstImage,
        width, 
        height
    );
}

void Buffer::copyBufferToImage(
    const vk::Device& device, 
    const vk::Queue& transferQueue,
    const vk::CommandPool& transferCommandPool,
    const vk::Buffer& srcBuffer, 
    const vk::Image& dstImage,
    const uint32_t& width, 
    const uint32_t& height)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif

    /// Create Command Buffer
    vk::CommandBuffer transferCommandBuffer =
        CommandBuffer::beginCommandBuffer(device, transferCommandPool);

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

    CommandBuffer::endAndSubmitCommandBuffer(
        device, 
        transferCommandPool, 
        transferQueue,
        transferCommandBuffer);
}

void Buffer::cpuUpdateBuffer(
    VmaAllocator& vma,
    VmaAllocation& memory, 
    const vk::DeviceSize& bufferSize, 
    void* dataStream)
{
    // Map memory
    void* data{};
    if (vmaMapMemory(
        vma,
        memory,
        &data) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to map memory using VMA.");
    }

    // Copy
    memcpy(
        data,
        dataStream,
        (size_t)bufferSize
    );

    // Unmap
    vmaUnmapMemory(
        vma,
        memory
    );
}
