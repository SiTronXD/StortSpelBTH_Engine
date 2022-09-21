#pragma once

#include "TempPCH.hpp"

class Device;

struct CreateBufferData
{
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
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
public:
	Buffer();
	~Buffer();

    static void createBuffer(CreateBufferData&& bufferData);

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
};