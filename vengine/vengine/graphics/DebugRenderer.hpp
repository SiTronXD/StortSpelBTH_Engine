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

struct DebugMeshPushConstantData
{
    glm::mat4 transform;
    glm::vec4 color;
};

struct DebugMeshDrawCallData
{
    uint32_t meshID;
    DebugMeshPushConstantData pushConstantData;
};

class DebugRenderer
{
private:
    friend VulkanRenderer;

    const uint32_t START_NUM_MAX_ELEMENTS = 64;

    std::vector<DebugMeshDrawCallData> meshDrawData;

    // Vertex buffers
    VertexStreams lineVertexStreams{};
    VertexBufferArray lineVertexBuffers;

    // Shader inputs
    UniformBufferID lineViewProjectionUB;
    ShaderInput lineShaderInput;
    Pipeline linePipeline;

    UniformBufferID meshViewProjectionUB;
    ShaderInput meshShaderInput;
    Pipeline meshPipeline;

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

    uint32_t sphereMeshID;
    uint32_t boxMeshID;

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
    void renderSphere(
        const glm::vec3& position,
        const float& radius,
        const glm::vec3& color);
    void renderBox(
        const glm::vec3& position,
        const glm::vec3& size,
        const glm::vec3& color);

    inline const std::vector<DebugMeshDrawCallData>& getMeshDrawCallData() const { return this->meshDrawData; }
    inline ShaderInput& getLineShaderInput() { return this->lineShaderInput; }
    inline ShaderInput& getMeshShaderInput() { return this->meshShaderInput; }
    inline const Pipeline& getLinePipeline() const { return this->linePipeline; }
    inline const Pipeline& getMeshPipeline() const { return this->meshPipeline; }
    inline const VertexBufferArray& getLineVertexBufferArray() const { return this->lineVertexBuffers; }
    inline size_t getNumVertices() { return this->numVertices; }
};