#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "vulkan/VmaUsage.hpp"
#include "ShaderInput.hpp"
#include "vulkan/Pipeline.hpp"
#include "VertexBufferArray.hpp"

class PhysicalDevice;
class Device;
class ResourceManager;
class VulkanRenderer;

class DebugRenderer
{
private:
    friend VulkanRenderer;

    const uint32_t START_NUM_MAX_ELEMENTS = 64;

    // Vertex buffers
    VertexStreams vertexStreams{};
    VertexBufferArray vertexBuffers;

    // Shader input
    UniformBufferID viewProjectionUB;
    ShaderInput shaderInput;
    Pipeline pipeline;

    // Vulkan
    PhysicalDevice* physicalDevice;
    Device* device;
    VmaAllocator* vma;
    ResourceManager* resourceManager;
    vk::RenderPass* renderPass;
    vk::Queue* transferQueue;
    vk::CommandPool* transferCommandPool;

    uint32_t numVertices;
    uint32_t framesInFlight;

    void prepareGPU(const uint32_t& currentFrame);
    void resetRender();

public:
    DebugRenderer();

    void create(
        PhysicalDevice& physicalDevice,
        Device& device,
        VmaAllocator& vma,
        ResourceManager& resourceManager,
        vk::RenderPass& renderPass,
        vk::Queue& transferQueue,
        vk::CommandPool& transferCommandPool,
        const uint32_t& framesInFlight);
    void initForScene();
    void cleanup();

    void renderLine(
        const glm::vec3& pos0, 
        const glm::vec3& pos1,
        const glm::vec3& color);

    inline ShaderInput& getShaderInput() { return this->shaderInput; }
    inline const Pipeline& getPipeline() const { return this->pipeline; }
    inline const VertexBufferArray& getVertexBufferArray() const { return this->vertexBuffers; }
    inline size_t getNumVertices() { return this->numVertices; }
};