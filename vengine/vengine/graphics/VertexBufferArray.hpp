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

	Device* device;
	VmaAllocator* vma;
	vk::Queue* transferQueue;
	vk::CommandPool* transferCommandPool;

public:
	VertexBufferArray();
	VertexBufferArray(VertexBufferArray&& ref);

	void create(
		Device& device,
		VmaAllocator& vma,
		vk::Queue& transferQueue,
		vk::CommandPool& transferCommandPool);

    // TODO: make this look better
	template <class T>
	void addVertexBuffer(
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

        BufferCreateData stagingBufferCreateData =
        {
            .bufferSize = bufferSize,
            .bufferUsageFlags = vk::BufferUsageFlagBits::eTransferSrc,       // This buffers vertex data will be transfered somewhere else!
            .bufferProperties = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .buffer = &stagingBuffer,
            .bufferMemory = &stagingBufferMemory,
            .allocationInfo = &allocInfo_staging,
            .vma = this->vma
        };
        Buffer::createBuffer(std::move(stagingBufferCreateData));

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
        BufferCreateData bufferCreateData = 
        {
            bufferSize,
            vk::BufferUsageFlagBits::eTransferDst        // Destination Buffer to be transfered to
                                | vk::BufferUsageFlagBits::eVertexBuffer,    // This is a Vertex Buffer
            VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            &this->vertexBuffers[this->vertexBuffers.size() - 1],
            &this->vertexBufferMemories[this->vertexBufferMemories.size() - 1],
            &allocInfo_deviceOnly,
            this->vma
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

	void cleanup();

	inline const size_t getNumVertexBuffers() const { return this->vertexBuffers.size(); }
	inline const std::vector<vk::Buffer>& getVertexBuffers() const
	{ return this->vertexBuffers; }
};