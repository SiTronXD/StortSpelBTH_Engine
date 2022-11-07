#include "pch.h"
#include "CommandBufferArray.hpp"
#include "Device.hpp"
#include "VulkanDbg.hpp"

void CommandBufferArray::createCommandBuffers(
    Device& device,
	vk::CommandPool& commandPool,
    const uint32_t& numCommandBuffers)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //:NOLINT
#endif

    // Number of command buffers
    this->commandBuffers.resize(numCommandBuffers);

    vk::CommandBufferAllocateInfo cbAllocInfo;
    cbAllocInfo.setCommandPool(commandPool);
    cbAllocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    cbAllocInfo.setCommandBufferCount(static_cast<uint32_t>(
        this->commandBuffers.size()));

    // Allocate command Buffers and place handles in array of buffers
    std::vector<vk::CommandBuffer> vkCommandBuffers = 
        device.getVkDevice().allocateCommandBuffers(cbAllocInfo);

    // Apply vk::CommandBuffer to command buffer wrappers
    for (size_t i = 0; i < this->commandBuffers.size(); i++)
    {
        this->commandBuffers[i].setVkCommandBuffer(vkCommandBuffers[i]);

        VulkanDbg::registerVkObjectDbgInfo("Graphics CommandBuffer[" + std::to_string(i) + "]", vk::ObjectType::eCommandBuffer, reinterpret_cast<uint64_t>(vk::CommandBuffer::CType(vkCommandBuffers[i])));
    }
}