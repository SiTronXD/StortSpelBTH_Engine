#include "VertexBufferArray.hpp"
#include "vulkan/Device.hpp"

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

template <typename T>
void VertexBufferArray::addVertexBuffer(
    const std::vector<T>& dataStream)
{
    // Empty data stream
    if (dataStream.size() <= 0)
        return;

    this->vertexBuffers.push_back(vk::Buffer());
    this->vertexBufferMemories.push_back(VmaAllocation());

    // Temporary buffer to "Stage" vertex data before transferring to GPU
    vk::Buffer stagingBuffer;
    VmaAllocation stagingBufferMemory{};
    VmaAllocationInfo allocInfo_staging;

    vk::DeviceSize bufferSize = sizeof(dataStream[0]) * dataStream.size();

    Buffer::createBuffer(
        {
            .bufferSize = bufferSize,
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,       // This buffers vertex data will be transfered somewhere else!
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer = &stagingBuffer,
            .bufferMemory = &stagingBufferMemory,
            .allocationInfo = &allocInfo_staging,
            .vma = *this->vma
        });

    // -- Map memory to our Temporary Staging Vertex Buffer -- 
    void* data{};
    if (vmaMapMemory(*this->vma, stagingBufferMemory, &data) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate Mesh Staging Vertex Buffer Using VMA!");
    };

    memcpy(
        data,
        dataStream.data(),
        (size_t)bufferSize
    );

    vmaUnmapMemory(*this->vma, stagingBufferMemory);

    VmaAllocationInfo allocInfo_deviceOnly;
    // Create Buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
    // Buffer memory is to be DEVICVE_LOCAL_BIT meaning memory is on the GPU and only accesible by it and not the CPU (HOST)
    Buffer::createBuffer(
        {
            .bufferSize = bufferSize,
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        // Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eVertexBuffer,    // This is a Vertex Buffer
            .bufferProperties = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .buffer = &this->vertexBuffers[this->vertexBuffers.size() - 1],
            .bufferMemory = &this->vertexBufferMemories[this->vertexBufferMemories.size() - 1],
            .allocationInfo = &allocInfo_deviceOnly,
            .vma = *this->vma

        });

    // Copy staging buffer to vertex buffer on GPU
    Buffer::copyBuffer(
        this->device->getVkDevice(),
        *this->transferQueue,
        *this->transferCommandPool,
        stagingBuffer,
        this->vertexBuffers[this->vertexBuffers.size() - 1],
        bufferSize);

    // Clean up staging buffer
    this->device->getVkDevice().destroyBuffer(stagingBuffer);
    vmaFreeMemory(*this->vma, stagingBufferMemory);
}

void VertexBufferArray::cleanup()
{
    for (size_t i = 0; i < this->vertexBuffers.size(); ++i)
    {
        this->device->destroyBuffer(this->vertexBuffers[i]);
        vmaFreeMemory(*this->vma, this->vertexBufferMemories[i]);
    }
}