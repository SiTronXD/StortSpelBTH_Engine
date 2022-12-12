#pragma once

#include <vector>
#include "Buffer.hpp"
#include "vulkan/Device.hpp"

class VertexBufferArray
{
private:
	// pos_0, uv_0, pos_1, uv_1, ... , pos_FiF-1, uv_FiF-1
	std::vector<vk::Buffer> vertexBuffers;
	std::vector<VmaAllocation> vertexBufferMemories;
    std::vector<vk::DeviceSize> vertexBufferOffsets;

	Device* device;
	VmaAllocator* vma;
	vk::Queue* transferQueue;
	vk::CommandPool* transferCommandPool;

    uint32_t framesInFlight;

public:
	VertexBufferArray();
	VertexBufferArray(VertexBufferArray&& ref);

	void create(
		Device& device,
		VmaAllocator& vma,
		vk::Queue& transferQueue,
		vk::CommandPool& transferCommandPool);
    void createForCpu(
        Device& device,
        VmaAllocator& vma,
        vk::Queue& transferQueue,
        vk::CommandPool& transferCommandPool,
        const uint32_t& framesInFlight);

	template <class T>
    void addVertexBuffer(
        const std::vector<T>& dataStream);

    template <class T>
    void addCpuVertexBuffer(
        const std::vector<T>& dataStream);

    template <class T>
    void cpuUpdate(
        const uint32_t& vertexBufferOffset,
        const uint32_t& frame,
        const std::vector<T>& dataStream);

	void cleanup();

    inline const uint32_t getVertexBufferFifOffset(const uint32_t& currentFrame) const 
    { return (currentFrame % this->framesInFlight) * static_cast<uint32_t>(this->getNumVertexBuffers()); }

    inline const size_t getNumVertexBuffers() const 
    { return this->vertexBuffers.size() / this->framesInFlight; }

    inline const std::vector<vk::DeviceSize>& getVertexBufferOffsets() const
    { return this->vertexBufferOffsets; }

	inline const std::vector<vk::Buffer>& getVertexBuffers() const
	{ return this->vertexBuffers; }
};

template <class T>
void VertexBufferArray::addVertexBuffer(
    const std::vector<T>& dataStream)
{
    // Empty data stream
    if (dataStream.size() <= 0)
        return;

    this->vertexBuffers.push_back(vk::Buffer());
    this->vertexBufferMemories.push_back(VmaAllocation());
    this->vertexBufferOffsets.push_back(0);

    // Temporary buffer to "stage" vertex data before transferring to GPU
    vk::Buffer stagingBuffer;
    VmaAllocation stagingBufferMemory{};
    VmaAllocationInfo allocInfoStaging;

    vk::DeviceSize bufferSize = sizeof(dataStream[0]) * dataStream.size();

    BufferCreateData stagingBufferCreateData =
    {
        .bufferSize = bufferSize,
        .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,       // This buffers vertex data will be transfered somewhere else!
        .bufferAllocationFlags = 
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
            VMA_ALLOCATION_CREATE_MAPPED_BIT |
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, // Dedicated memory can be completely freed, and therefore saves cpu RAM afterwards.
        .buffer = &stagingBuffer,
        .bufferMemory = &stagingBufferMemory,
        .allocationInfo = &allocInfoStaging,
        .vma = this->vma
    };
    Buffer::createBuffer(std::move(stagingBufferCreateData));

    // Update memory in staging buffer
    Buffer::cpuUpdateBuffer(
        *this->vma,
        stagingBufferMemory, 
        bufferSize, 
        (void*) dataStream.data()
    );

    VmaAllocationInfo allocInfo_deviceOnly{};
    // Create Buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also VERTEX_BUFFER)
    // Buffer memory is to be DEVICVE_LOCAL_BIT meaning memory is on the GPU and only accesible by it and not the CPU (HOST)
    BufferCreateData bufferCreateData =
    {
        .bufferSize = bufferSize,
        .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferDst        // Destination Buffer to be transfered to
                            | vk::BufferUsageFlagBits::eVertexBuffer,    // This is a Vertex Buffer
        .bufferAllocationFlags = 0,
        .buffer = &this->vertexBuffers[this->vertexBuffers.size() - 1],
        .bufferMemory = &this->vertexBufferMemories[this->vertexBufferMemories.size() - 1],
        .allocationInfo = &allocInfo_deviceOnly,
        .vma = this->vma
    };
    Buffer::createBuffer(std::move(bufferCreateData));

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

template <class T>
void VertexBufferArray::addCpuVertexBuffer(
    const std::vector<T>& dataStream)
{
    // Empty data stream
    if (dataStream.size() <= 0)
        return;

    // Create 1 buffer per frame in flight
    std::vector<vk::Buffer> createdBuffers(this->framesInFlight);
    std::vector<VmaAllocation> createdBufferMemories(this->framesInFlight);

    vk::DeviceSize bufferSize = sizeof(dataStream[0]) * dataStream.size();

    // Create 1 buffer per frame in flight
    for (uint32_t i = 0; i < this->framesInFlight; ++i)
    {
        VmaAllocationInfo allocInfo_deviceOnly{};
        BufferCreateData bufferCreateData =
        {
            bufferSize,
            vk::BufferUsageFlagBits::eTransferDst        // Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eVertexBuffer,    // This is a Vertex Buffer
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            &createdBuffers[i],
            &createdBufferMemories[i],
            &allocInfo_deviceOnly,
            this->vma
        };
        Buffer::createBuffer(std::move(bufferCreateData));

        // Copy memory to vertex buffer
        Buffer::cpuUpdateBuffer(
            *this->vma,
            createdBufferMemories[i], 
            bufferSize, 
            (void*) dataStream.data()
        );
    }

    // Insert created buffers into actual buffer list
    uint32_t numCpuVertexBuffers =
        static_cast<uint32_t>(this->vertexBuffers.size()) /
        this->framesInFlight;
    uint32_t currentOffset = numCpuVertexBuffers;
    for (uint32_t i = 0; i < this->framesInFlight; ++i)
    {
        // Vertex buffer
        this->vertexBuffers.insert(
            this->vertexBuffers.begin() + currentOffset,
            createdBuffers[i]
        );
        // Vertex buffer memory
        this->vertexBufferMemories.insert(
            this->vertexBufferMemories.begin() + currentOffset,
            createdBufferMemories[i]
        );
        // Vertex buffer offset (order doesn't matter)
        this->vertexBufferOffsets.push_back(0);

        // Move offset
        currentOffset += numCpuVertexBuffers + 1;
    }
}

template <class T>
void VertexBufferArray::cpuUpdate(
    const uint32_t& vertexBufferOffset,
    const uint32_t& frame,
    const std::vector<T>& dataStream)
{
    uint32_t i = this->getNumVertexBuffers() * frame + vertexBufferOffset;
    vk::DeviceSize bufferSize = sizeof(dataStream[0]) * dataStream.size();

    // Copy data to vertex buffer
    Buffer::cpuUpdateBuffer(
        *this->vma, 
        this->vertexBufferMemories[i], 
        bufferSize, 
        (void*) dataStream.data()
    );
}