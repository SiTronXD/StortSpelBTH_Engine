#include "VertexBufferArray.hpp"

VertexBufferArray::VertexBufferArray()
    : device(nullptr), 
    vma(nullptr),
    transferQueue(nullptr),
    transferCommandPool(nullptr),
    areCpuBuffers(false)
{

}

VertexBufferArray::VertexBufferArray(VertexBufferArray&& ref)
    : device(ref.device),
    vma(ref.vma),
    transferQueue(ref.transferQueue),
    transferCommandPool(ref.transferCommandPool),
    framesInFlight(ref.framesInFlight),
    areCpuBuffers(ref.areCpuBuffers),
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
        0 // Won't be used for GPU only vertex buffers
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
}