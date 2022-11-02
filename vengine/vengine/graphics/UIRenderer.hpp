#pragma once

#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "vulkan/VmaUsage.hpp"
#include "ShaderInput.hpp"
#include "vulkan/Pipeline.hpp"

class PhysicalDevice;
class Device;
class ResourceManager;
class VulkanRenderer;

struct UIElementData
{
    glm::vec4 transform;    // vec4(x, y, scaleX, scaleY)
    glm::vec4 uvRect;       // vec4(u, v, sizeU, sizeV)
};

struct UIDrawCallData
{
    uint32_t textureIndex;
    uint32_t startVertex;
    uint32_t numVertices;
};

struct CharacterRect
{
    float x;
    float y;
    float width;
    float height;
};

class UIRenderer
{
private:
    friend VulkanRenderer;

    const uint32_t START_NUM_MAX_ELEMENTS = 256;

    std::unordered_map<char, CharacterRect> characterRects;

    std::vector<UIElementData> uiElementData;
    std::vector<UIDrawCallData> uiDrawCallData;

    SamplerID uiSamplerID;
    StorageBufferID storageBufferID;
    ShaderInput uiShaderInput;
    Pipeline uiPipeline;

    uint32_t currentElementIndex;
    uint32_t uiTextureIndex;

    float uiTextureWidth;
    float uiTextureHeight;
    float oneOverCharTileWidth;
    float oneOverCharTileHeight;

    uint32_t framesInFlight;

    PhysicalDevice* physicalDevice;
    Device* device;
    VmaAllocator* vma;
    ResourceManager* resourceManager;
    vk::RenderPass* renderPass;

    void prepareForGPU();
    void resetRender();

public:
    UIRenderer();

    void create(
        PhysicalDevice& physicalDevice,
        Device& device,
        VmaAllocator& vma,
        ResourceManager& resourceManager,
        vk::RenderPass& renderPass,
        const uint32_t& framesInFlight);
    void initForScene();
    void cleanup();
    void setBitmapFont(
        const std::vector<std::string>& characters,
        const uint32_t& bitmapFontTextureIndex,
        const uint32_t& tileWidth,
        const uint32_t& tileHeight);

    void setTexture(const uint32_t& textureIndex);
    void renderTexture(
        const float& x,
        const float& y,
        const float& width,
        const float& height,
        const float& u0 = 0.0f,
        const float& v0 = 0.0f,
        const float& u1 = 1.0f,
        const float& v1 = 1.0f
    );
    void renderString(
        const std::string& text,
        const float& x,
        const float& y,
        const float& characterWidth,
        const float& characterHeight
    );

    inline const StorageBufferID& getStorageBufferID() const { return this->storageBufferID; }
    inline const SamplerID& getSamplerID() const { return this->uiSamplerID; }
    inline std::vector<UIElementData>& getUiElementData() { return this->uiElementData; }
    inline const std::vector<UIDrawCallData>& getUiDrawCallData() const { return this->uiDrawCallData; }
    inline const uint32_t& getUiTextureIndex() const { return this->uiTextureIndex; }
    inline ShaderInput& getShaderInput() { return this->uiShaderInput; }
    inline const Pipeline& getPipeline() const { return this->uiPipeline; }

    // Mostly for Lua use
    inline ResourceManager* getResourceManager() const { return this->resourceManager; }
};