#include "pch.h"
#include "UniformBuffer.hpp"

void UniformBuffer::createUniformBuffer(
    Device& device,
    VmaAllocator& vma,
    const size_t& bufferSize,
    const uint32_t& framesInFlight)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif
    Buffer::init(device, vma, bufferSize);

    std::vector<vk::Buffer>& buffers = Buffer::getBuffers();
    std::vector<VmaAllocation>& bufferMemories = Buffer::getBufferMemories();

    // One uniform buffer for each frame in flight
    buffers.resize(framesInFlight);
    bufferMemories.resize(framesInFlight);
    std::vector<VmaAllocationInfo> bufferMemoriesInfo(framesInFlight);

    // Create Uniform Buffers 
    for (size_t i = 0; i < buffers.size(); i++)
    {
        Buffer::createBuffer(
            {
                .bufferSize = (vk::DeviceSize) bufferSize,
                .bufferUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer,         // We're going to use this as a Uniform Buffer...
                // .bufferProperties = vk::MemoryPropertyFlagBits::eHostVisible         // So we can access the Data from the HOST (CPU)
                //                     | vk::MemoryPropertyFlagBits::eHostCoherent,     // So we don't have to flush the data constantly...
                .bufferAllocationFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .buffer = &buffers[i],
                .bufferMemory = &bufferMemories[i],
                .allocationInfo = &bufferMemoriesInfo[i],
                .vma = &vma
            }
        );

        VulkanDbg::registerVkObjectDbgInfo("UniformBuffer[" + std::to_string(i) + "]", vk::ObjectType::eBuffer, reinterpret_cast<uint64_t>(vk::Buffer::CType(buffers[i])));
    }
}