#include "pch.h"
#include "CommandBuffer.hpp"
#include "Pipeline.hpp"
#include "../ShaderInput.hpp"
#include "../VertexBufferArray.hpp"

void CommandBuffer::begin(const vk::CommandBufferBeginInfo& beginInfo)
{
    this->commandBuffer.begin(beginInfo);
}

void CommandBuffer::beginOneTimeSubmit()
{
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    this->begin(beginInfo);
}

void CommandBuffer::end()
{
    this->commandBuffer.end();
}

void CommandBuffer::beginRenderPass2(
    const vk::RenderPassBeginInfo& renderPassBeginInfo,
    const vk::SubpassBeginInfoKHR& subpassBeginInfo)
{
    this->commandBuffer.beginRenderPass2(
        renderPassBeginInfo,
        subpassBeginInfo
    );
}

void CommandBuffer::endRenderPass2(const vk::SubpassEndInfo& subpassEndInfo)
{
    this->commandBuffer.endRenderPass2(subpassEndInfo);
}

void CommandBuffer::setViewport(const vk::Viewport& viewport)
{
    this->commandBuffer.setViewport(0, 1, &viewport);
}

void CommandBuffer::setScissor(const vk::Rect2D& scissor)
{
    this->commandBuffer.setScissor(0, 1, &scissor);
}

void CommandBuffer::bindPipeline(const Pipeline& pipeline)
{
    this->currentBindPoint = pipeline.getVkPipelineBindPoint();
    this->commandBuffer.bindPipeline(
        this->currentBindPoint,
        pipeline.getVkPipeline()
    );
}

void CommandBuffer::pushConstant(
    ShaderInput& shaderInput,
    void* data)
{
    this->commandBuffer.pushConstants(
        shaderInput.getPipelineLayout().getVkPipelineLayout(),
        shaderInput.getPushConstantShaderStage(),   // Stage to push the Push Constant to.
        uint32_t(0),                        // Offset of Push Constants to update; 
        shaderInput.getPushConstantSize(),  // Size of data being pushed
        data                        // Actual data being pushed (can also be an array)
    );
}

void CommandBuffer::bindVertexBuffers2(const vk::Buffer& vertexBuffer)
{
    const std::array<vk::Buffer, 1> vertexBufferArray = { vertexBuffer };
    const std::array<vk::DeviceSize, 1> offsets = { 0 };
    this->commandBuffer.bindVertexBuffers2(
        uint32_t(0),
        uint32_t(1),
        vertexBufferArray.data(),
        offsets.data(),
        nullptr,        //NOTE: Could also be a pointer to an array of the size in bytes of vertex data bound from pBuffers (vertexBuffer)
        nullptr         //NOTE: Could also be a pointer to an array of buffer strides
    );
}

void CommandBuffer::bindVertexBuffers2(
    const std::vector<vk::DeviceSize>& vertexBufferOffsets,
    const std::vector<vk::Buffer>& vertexBuffers)
{
    this->commandBuffer.bindVertexBuffers2(
        uint32_t(0),
        static_cast<uint32_t>(vertexBuffers.size()),
        vertexBuffers.data(),
        vertexBufferOffsets.data(),
        nullptr,        //NOTE: Could also be a pointer to an array of the size in bytes of vertex data bound from pBuffers (vertexBuffer)
        nullptr         //NOTE: Could also be a pointer to an array of buffer strides
    );
}

void CommandBuffer::bindVertexBuffers2(
    const VertexBufferArray& vertexBufferArray,
    const uint32_t& currentFrame)
{
    uint32_t vertexBufferFifOffset =
        vertexBufferArray.getVertexBufferFifOffset(currentFrame);
    uint32_t numVertexBuffers = vertexBufferArray.getNumVertexBuffers();

    this->commandBuffer.bindVertexBuffers2(
        uint32_t(0),
        numVertexBuffers,
        &vertexBufferArray.getVertexBuffers()[vertexBufferFifOffset],
        vertexBufferArray.getVertexBufferOffsets().data(),
        nullptr,        //NOTE: Could also be a pointer to an array of the size in bytes of vertex data bound from pBuffers (vertexBuffer)
        nullptr         //NOTE: Could also be a pointer to an array of buffer strides
    );
}

void CommandBuffer::bindIndexBuffer(const vk::Buffer& indexBuffer)
{
    this->commandBuffer.bindIndexBuffer(
        indexBuffer,
        0,
        vk::IndexType::eUint32
    );
}

void CommandBuffer::bindShaderInputFrequency(
    const ShaderInput& shaderInput,
    const DescriptorFrequency& descriptorFrequency)
{
    const uint32_t frequencyIndex = (uint32_t)descriptorFrequency;

    const std::vector<vk::DescriptorSet*>& descriptorSetGroup =
        shaderInput.getBindDescriptorSets();

    // Bind descriptor set, if it can
    if (descriptorSetGroup[frequencyIndex] != nullptr)
    {
        this->commandBuffer.bindDescriptorSets(
            this->currentBindPoint,
            shaderInput.getPipelineLayout().getVkPipelineLayout(),            // The Pipeline Layout that describes how the data will be accessed in our shaders
            frequencyIndex,                               // Which Set is the first we want to use? 
            1, // How many Descriptor Sets where going to go through?
            descriptorSetGroup[frequencyIndex],                       // The Descriptor Set to be used (Remember, 1:1 relationship with CommandBuffers/Images)
            0,                               // Dynamic Offset Count;  we dont Dynamic Uniform Buffers use anymore...
            nullptr);                        // Dynamic Offset;        We dont use Dynamic Uniform Buffers  anymore...
    }
}

void CommandBuffer::draw(
    const uint32_t& vertexCount,
    const uint32_t& instanceCount,
    const uint32_t& firstVertex,
    const uint32_t& firstInstance)
{
    this->commandBuffer.draw(
        vertexCount, 
        instanceCount,
        firstVertex, 
        firstInstance
    );
}

void CommandBuffer::drawIndexed(
    const uint32_t& indexCount,
    const uint32_t& instanceCount,
    const uint32_t& firstIndex,
    const uint32_t& vertexOffset,
    const uint32_t& firstInstance)
{
    this->commandBuffer.drawIndexed(
        indexCount,
        instanceCount,
        firstIndex,
        vertexOffset,
        firstInstance
    );
}

void CommandBuffer::setVkCommandBuffer(
    const vk::CommandBuffer& commandBuffer)
{
    this->commandBuffer = commandBuffer;
}

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