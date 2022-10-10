#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "vulkan/VmaUsage.hpp"
#include "ShaderInput.hpp"
#include "vulkan/Pipeline.hpp"

class PhysicalDevice;
class Device;
class ResourceManager;

class UIRenderer
{
private:
    const uint32_t START_NUM_MAX_ELEMENTS = 2;

    SamplerID uiSamplerID;
    ShaderInput uiShaderInput;
    Pipeline uiPipeline;

    // One vertex buffer per frame in flight
    std::vector<vk::Buffer> vertexBuffers;
    std::vector<VmaAllocation> vertexBufferMemories;

    uint32_t currentElementIndex;
    uint32_t numRenderVerts;

    Device* device;
    VmaAllocator* vma;

    void createVertexBuffers(
        Device& device,
        VmaAllocator& vma,
        vk::Queue& transferQueue,
        vk::CommandPool& transferCommandPool,
        const uint32_t& framesInFlight);
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
        const float& x,
        const float& y,
        const float& width,
        const float& height
    );
    void endUI();

    inline const uint32_t& getNumRenderVerts() const { return this->numRenderVerts; }
    inline const vk::Buffer& getVertexBuffer(const uint32_t& index) const { return this->vertexBuffers[index]; }
    inline /*const*/ ShaderInput& getShaderInput() /*const*/ { return this->uiShaderInput; }
    inline const Pipeline& getPipeline() const { return this->uiPipeline; }
};