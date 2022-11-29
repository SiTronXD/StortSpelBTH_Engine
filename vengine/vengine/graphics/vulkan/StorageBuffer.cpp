#include "pch.h"
#include "StorageBuffer.hpp"

void StorageBuffer::createStorageBuffer(
    Device& device, 
    VmaAllocator& vma,
    const size_t& bufferSize,
    uint32_t framesInFlight,
    const bool& gpuOnly,
    void* initialData,
    vk::Queue* transferQueue,
    vk::CommandPool* transferCommandPool)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    Buffer::init(device, vma, bufferSize);

    // One single buffer if it is GPU only
    if (gpuOnly)
    {
        framesInFlight = 1;
    }

    std::vector<vk::Buffer>& buffers = Buffer::getBuffers();
    std::vector<VmaAllocation>& bufferMemories = Buffer::getBufferMemories();

    // One uniform buffer for each frame in flight
    buffers.resize(framesInFlight);
    bufferMemories.resize(framesInFlight);
    std::vector<VmaAllocationInfo> bufferMemoriesInfo(framesInFlight);

    if (!gpuOnly)
    {
        // Create storage buffers
        for (size_t i = 0; i < buffers.size(); ++i)
        {
            Buffer::createBuffer(
                {
                    .bufferSize = (vk::DeviceSize)bufferSize,
                    .bufferUsageFlags =
                        vk::BufferUsageFlagBits::eStorageBuffer,
                .bufferAllocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                    VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .buffer = &buffers[i],
                .bufferMemory = &bufferMemories[i],
                .allocationInfo = &bufferMemoriesInfo[i],
                .vma = &vma
                }
            );

            VulkanDbg::registerVkObjectDbgInfo(
                "StorageBuffer[" + std::to_string(i) + "]", vk::ObjectType::eBuffer,
                reinterpret_cast<uint64_t>(vk::Buffer::CType(buffers[i])));
        }
    }
    else
    {
        if (transferQueue == nullptr || transferCommandPool == nullptr)
        {
            Log::error("StorageBuffer::createStorageBuffer: queue and/or command pool are nullptr.");
            return;
        }

        vk::Buffer stagingBuffer{};
        VmaAllocation stagingBufferMemory{};
        VmaAllocationInfo allocInfoStaging;

        // Create staging buffer
        Buffer::createBuffer(
            {
                .bufferSize = (vk::DeviceSize)bufferSize,
                .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,
                .bufferAllocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .buffer = &stagingBuffer,
                .bufferMemory = &stagingBufferMemory,
                .allocationInfo = &allocInfoStaging,
                .vma = &vma
            }
        );

        // Update staging buffer
        Buffer::cpuUpdateBuffer(
            vma, 
            stagingBufferMemory, 
            bufferSize, 
            initialData
        );

        // Create gpu buffer
        VmaAllocationInfo allocInfo;
        Buffer::createBuffer(
            {
                .bufferSize = bufferSize,
                .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst
                                    | vk::BufferUsageFlagBits::eStorageBuffer,
                .bufferAllocationFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                .buffer = &buffers[0],
                .bufferMemory = &bufferMemories[0],
                .allocationInfo = &allocInfo,
                .vma = &vma
            }
        );

        Buffer::copyBuffer(
            device.getVkDevice(),
            *transferQueue,
            *transferCommandPool,
            stagingBuffer,
            buffers[0],
            bufferSize
        );

        // Destroy/free staging buffer resources
        device.getVkDevice().destroyBuffer(stagingBuffer);
        vmaFreeMemory(vma, stagingBufferMemory);
    }
}