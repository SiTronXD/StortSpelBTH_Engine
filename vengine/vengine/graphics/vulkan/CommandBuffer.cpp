#include "CommandBuffer.hpp"

CommandBuffer::CommandBuffer()
{}

CommandBuffer::~CommandBuffer()
{}

vk::CommandBuffer CommandBuffer::beginCommandBuffer(vk::Device device,
    vk::CommandPool commandPool)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif
    // Command Buffer to hold transfer Commands
    vk::CommandBuffer commandBuffer;

    // Commaand Buffer Details
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
    allocInfo.setCommandPool(commandPool);
    allocInfo.setCommandBufferCount(uint32_t(1));

    // Allocate CommandBuffer from pool
    commandBuffer = device.allocateCommandBuffers(allocInfo)[0];

    // Information to begin the command buffer Record
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(
        vk::CommandBufferUsageFlagBits::
        eOneTimeSubmit); // This command buffer is only used once, so set
    // it up for one submit

// Begin recording Transfer Commands
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void CommandBuffer::endAndSubmitCommandBuffer(
    vk::Device device,
    vk::CommandPool commandPool,
    vk::Queue queue,
    vk::CommandBuffer commandBuffer)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif

    commandBuffer.end();

    // Queue Submission information
    vk::SubmitInfo2 submitInfo;
    vk::CommandBufferSubmitInfo commandbufferSubmitInfo;
    commandbufferSubmitInfo.setCommandBuffer(commandBuffer);
    submitInfo.setCommandBufferInfoCount(uint32_t(1));
    submitInfo.setPCommandBufferInfos(&commandbufferSubmitInfo);

    // Submit Transfer Commands to transfer Queue and wait for it to finish
    queue.submit2(submitInfo);

    queue.waitIdle(); // TODO: this is bad, use proper synchronization
    // instead!

// Free temporary Command Buffer back to pool
    device.freeCommandBuffers(commandPool, uint32_t(1), &commandBuffer);
}

void CommandBuffer::endAndSubmitCommandBufferWithFences(
    vk::Device device, vk::CommandPool commandPool, vk::Queue queue,
    vk::CommandBuffer commandBuffer)
{
#ifndef VENGINE_NO_PROFILING
    ZoneScoped; //: NOLINT
#endif
    uint64_t const fence_timeout_ms = 100000000000;
    commandBuffer.end();

    // Queue Submission information
    vk::SubmitInfo2 submitInfo;
    vk::CommandBufferSubmitInfo commandbufferSubmitInfo;
    commandbufferSubmitInfo.setCommandBuffer(commandBuffer);
    submitInfo.setCommandBufferInfoCount(uint32_t(1));
    submitInfo.setPCommandBufferInfos(&commandbufferSubmitInfo);

    vk::FenceCreateInfo fenceInfo;
    fenceInfo.setFlags(
        vk::FenceCreateFlagBits::eSignaled); // TODO:: this must be right,
    // enum has only one value...
    vk::Fence fence = device.createFence(fenceInfo);

    // Submit Transfer Commands to transfer Queue and wait for it to finish
    queue.submit2(submitInfo, fence);

    if (device.waitForFences(fence, VK_TRUE, fence_timeout_ms) !=
        vk::Result::eSuccess) {
        throw std::runtime_error("Failed at waiting for a Fence!");
    }
    device.destroyFence(fence);

    // Free temporary Command Buffer back to pool
    device.freeCommandBuffers(commandPool, commandBuffer);
}