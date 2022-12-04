#pragma once 
 #include "op_overload.hpp"

#include <array>
#include "../TempPCH.hpp"

class Pipeline;
class PipelineLayout;
class ShaderInput;
class VertexBufferArray;
enum class DescriptorFrequency;

class CommandBuffer
{
private:
    vk::PipelineBindPoint currentBindPoint;

    vk::CommandBuffer commandBuffer;

public:
    void begin(const vk::CommandBufferBeginInfo& beginInfo);
    void beginOneTimeSubmit();
    void end();
    void beginRenderPass2(
        const vk::RenderPassBeginInfo& renderPassBeginInfo,
        const vk::SubpassBeginInfoKHR& subpassBeginInfo);
    void endRenderPass2(const vk::SubpassEndInfo& subpassEndInfo);

    void setViewport(const vk::Viewport& viewport);
    void setScissor(const vk::Rect2D& scissor);
    void bindPipeline(const Pipeline& pipeline);
    void pushConstant(
        ShaderInput& shaderInput,
        void* data);

    void bindVertexBuffers2(const vk::Buffer& vertexBuffer);
    void bindVertexBuffers2(
        const std::vector<vk::DeviceSize>& vertexBufferOffsets,
        const std::vector<vk::Buffer>& vertexBuffers);
    void bindVertexBuffers2(
        const VertexBufferArray& vertexBufferArray,
        const uint32_t& currentFrame);

    void bindIndexBuffer(const vk::Buffer& indexBuffer);

    void bindShaderInputFrequency(
        const ShaderInput& shaderInput,
        const DescriptorFrequency& descriptorFrequency);

    void draw(
        const uint32_t& vertexCount,
        const uint32_t& instanceCount = 1,
        const uint32_t& firstVertex = 0,
        const uint32_t& firstInstance = 0);
    void drawIndexed(
        const uint32_t& indexCount,
        const uint32_t& instanceCount = 1,
        const uint32_t& firstIndex = 0,
        const uint32_t& vertexOffset = 0,
        const uint32_t& firstInstance = 0);

    void setVkCommandBuffer(
        const vk::CommandBuffer& commandBuffer);

    static vk::CommandBuffer beginCommandBuffer(
        vk::Device device,
        vk::CommandPool commandPool);

    static void endAndSubmitCommandBuffer(
        vk::Device device,
        vk::CommandPool commandPool, vk::Queue queue,
        vk::CommandBuffer commandBuffer);

    static void endAndSubmitCommandBufferWithFences(
        vk::Device device, 
        vk::CommandPool commandPool, 
        vk::Queue queue,
        vk::CommandBuffer commandBuffer);

    inline vk::CommandBuffer& getVkCommandBuffer() 
    { return this->commandBuffer; }
};