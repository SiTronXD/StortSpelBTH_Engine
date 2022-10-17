#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include "vulkan/VmaUsage.hpp"
#include "ShaderInput.hpp"
#include "vulkan/Pipeline.hpp"

class PhysicalDevice;
class Device;
class ResourceManager;

class DebugRenderer
{
private:
    const uint32_t START_NUM_MAX_ELEMENTS = 64;

    ShaderInput shaderInput;
    Pipeline pipeline;

    PhysicalDevice* physicalDevice;
    Device* device;
    VmaAllocator* vma;
    ResourceManager* resourceManager;
    vk::RenderPass* renderPass;

    uint32_t framesInFlight;

public:
    DebugRenderer();

    void create(
        PhysicalDevice& physicalDevice,
        Device& device,
        VmaAllocator& vma,
        ResourceManager& resourceManager,
        vk::RenderPass& renderPass,
        const uint32_t& framesInFlight);
    void initForScene();
    void cleanup();

    inline ShaderInput& getShaderInput() { return this->shaderInput; }
    inline const Pipeline& getPipeline() const { return this->pipeline; }
};