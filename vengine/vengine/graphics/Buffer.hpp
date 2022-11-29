#pragma once

#include "TempPCH.hpp"

class Device;

struct BufferCreateData
{
    vk::DeviceSize bufferSize;
    vk::BufferUsageFlags bufferUsageFlags;
    VmaAllocationCreateFlags bufferProperties; //TODO: Replace this name... (!!)
    vk::Buffer* buffer;
    VmaAllocation* bufferMemory;
    VmaAllocationInfo* allocationInfo = nullptr;
    VmaAllocator* vma;
};

class Buffer
{
private:
    std::vector<vk::Buffer> buffers;
    std::vector<VmaAllocation> bufferMemories;
    // std::vector<VmaAllocationInfo> bufferMemoryInfos;

    Device* device;
    VmaAllocator* vma;

    size_t bufferSize;

protected:
    void map(void*& outputWriteData, const uint32_t& bufferIndex);
    void unmap(const uint32_t& bufferIndex);

    inline std::vector<vk::Buffer>& getBuffers() { return this->buffers; }
    inline std::vector<VmaAllocation>& getBufferMemories() { return this->bufferMemories; }

public:
	Buffer();
	virtual ~Buffer();

    void init(
        Device& device,
        VmaAllocator& vma,
        const size_t& bufferSize
    );
    void update(void* copyData, const uint32_t& currentFrame);
    void cleanup();

    inline const size_t& getBufferSize() { return this->bufferSize; }

    static void createBuffer(BufferCreateData&& bufferData);

    static void copyBuffer(
        Device& device,
        const vk::Queue& transferQueue,
        const vk::CommandPool& transferCommandPool,
        const vk::Buffer& srcBuffer,
        const vk::Buffer& dstBuffer,
        const vk::DeviceSize& bufferSize);

    static void copyBuffer(
        vk::Device& device,
        const vk::Queue& transferQueue,
        const vk::CommandPool& transferCommandPool,
        const vk::Buffer& srcBuffer,
        const vk::Buffer& dstBuffer,
        const vk::DeviceSize& bufferSize);

    static void copyBufferToImage(
        Device& device,
        const vk::Queue& transferQueue,
        const vk::CommandPool& transferCommandPool,
        const vk::Buffer& srcBuffer,
        const vk::Image& dstImage,
        const uint32_t& width,
        const uint32_t& height);

    static void copyBufferToImage(
        const vk::Device& device,
        const vk::Queue& transferQueue,
        const vk::CommandPool& transferCommandPool,
        const vk::Buffer& srcBuffer,
        const vk::Image& dstImage,
        const uint32_t& width,
        const uint32_t& height);

    inline vk::Buffer& getBuffer(const uint32_t& index) { return this->buffers[index % this->buffers.size()]; }
    inline size_t getNumBuffers() const { return this->buffers.size(); }    
};