#include "VertexBufferArray.hpp"

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
    vertexBuffers(std::move(ref.vertexBuffers)),
    vertexBufferMemories(std::move(ref.vertexBufferMemories))
{

}

void VertexBufferArray::create(
    Device& device,
    VmaAllocator& vma,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool)
{
    this->device = &device;
    this->vma = &vma;
    this->transferQueue = &transferQueue;
    this->transferCommandPool = &transferCommandPool;
}

void VertexBufferArray::cleanup()
{
    for (size_t i = 0; i < this->vertexBuffers.size(); ++i)
    {
        this->device->destroyBuffer(this->vertexBuffers[i]);
        vmaFreeMemory(*this->vma, this->vertexBufferMemories[i]);
    }
}