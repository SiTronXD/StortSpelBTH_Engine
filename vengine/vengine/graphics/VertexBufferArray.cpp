#include "pch.h"
#include "VertexBufferArray.hpp"

void VertexBufferArray::cpuUpdateBuffer(
    VmaAllocation& memory,
    const vk::DeviceSize& bufferSize,
    void* dataStream)
{
    // Map memory for transfer to vertex buffer
    void* data{};
    if (vmaMapMemory(
        *this->vma,
        memory,
        &data) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to map memory using VMA.");
    }

    memcpy(
        data,
        dataStream,
        (size_t)bufferSize
    );

    vmaUnmapMemory(
        *this->vma,
        memory
    );
}

VertexBufferArray::VertexBufferArray()
    : device(nullptr), 
    vma(nullptr),
    transferQueue(nullptr),
    transferCommandPool(nullptr)
{

}

VertexBufferArray::VertexBufferArray(VertexBufferArray&& ref)
    : device(ref.device),
    vma(ref.vma),
    transferQueue(ref.transferQueue),
    transferCommandPool(ref.transferCommandPool),
    framesInFlight(ref.framesInFlight),
    vertexBuffers(std::move(ref.vertexBuffers)),
    vertexBufferMemories(std::move(ref.vertexBufferMemories)),
    vertexBufferOffsets(std::move(ref.vertexBufferOffsets))
{

}

void VertexBufferArray::create(
    Device& device,
    VmaAllocator& vma,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool)
{
    this->createForCpu(
        device,
        vma,
        transferQueue,
        transferCommandPool,
        1 // Won't really be used for GPU only vertex buffers
    );
}

void VertexBufferArray::createForCpu(
    Device& device,
    VmaAllocator& vma,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool,
    const uint32_t& framesInFlight)
{
    this->device = &device;
    this->vma = &vma;
    this->transferQueue = &transferQueue;
    this->transferCommandPool = &transferCommandPool;
    this->framesInFlight = framesInFlight;
}

void VertexBufferArray::cleanup()
{
    for (size_t i = 0; i < this->vertexBuffers.size(); ++i)
    {
        this->device->destroyBuffer(this->vertexBuffers[i]);
        vmaFreeMemory(*this->vma, this->vertexBufferMemories[i]);
    }
    this->vertexBufferOffsets.clear();
    this->vertexBufferMemories.clear();
    this->vertexBuffers.clear();
}