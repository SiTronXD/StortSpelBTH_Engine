#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "vulkan/VmaUsage.hpp"
#include "ShaderInput.hpp"
#include "vulkan/Pipeline.hpp"

class PhysicalDevice;
class Device;
class ResourceManager;

struct UIElementData
{
    glm::vec4 subTextureRect;

    // vec4(x, y, scaleX, scaleY)
    glm::vec4 transform;
};

class UIRenderer
{
private:
    const uint32_t START_NUM_MAX_ELEMENTS = 2;

    std::vector<UIElementData> uiElementData;

    SamplerID uiSamplerID;
    StorageBufferID storageBufferID;
    ShaderInput uiShaderInput;
    Pipeline uiPipeline;

    // One vertex buffer per frame in flight
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<VmaAllocation> vertexBufferMemories;

    uint32_t currentElementIndex;
    uint32_t numRenderVerts;
    uint32_t uiTextureIndex;

    float uiTextureWidth;
    float uiTextureHeight;

    Device* device;
    VmaAllocator* vma;
    ResourceManager* resourceManager;

public:
    UIRenderer();

    void create(
        PhysicalDevice& physicalDevice,
        Device& device,
        VmaAllocator& vma,
        ResourceManager& resourceManager,
        vk::RenderPass& renderPass,
        vk::Queue& transferQueue,
        vk::CommandPool& transferCommandPool,
        const uint32_t& framesInFlight);

    void cleanup();

    void setUiTexture(const uint32_t& textureIndex);

    void beginUI();
    void renderTexture(
        glm::vec4 subTextureRect,
        const float& x,
        const float& y,
        const float& width,
        const float& height
    );
    void endUI();

    inline const StorageBufferID& getStorageBufferID() const { return this->storageBufferID; }
    inline const SamplerID& getSamplerID() const { return this->uiSamplerID; }
    inline std::vector<UIElementData>& getUiElementData() { return this->uiElementData; }
    inline const uint32_t& getNumRenderVerts() const { return this->numRenderVerts; }
    inline const uint32_t& getUiTextureIndex() const { return this->uiTextureIndex; }
    inline const vk::Buffer& getVertexBuffer(const uint32_t& index) const { return this->vertexBuffers[index]; }
    inline ShaderInput& getShaderInput() { return this->uiShaderInput; }
    inline const Pipeline& getPipeline() const { return this->uiPipeline; }
};