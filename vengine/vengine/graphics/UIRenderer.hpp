#pragma once

#include <vector>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "vulkan/VmaUsage.hpp"
#include "ShaderInput.hpp"
#include "vulkan/Pipeline.hpp"
#include "StringAlignment.h"

class PhysicalDevice;
class Device;
class ResourceManager;
class VulkanRenderer;
class SceneHandler;

struct UIElementData
{
    glm::vec4 transform;    // vec4(x, y, scaleX, scaleY)
    glm::uvec4 uvRect;       // ivec4(u, v, sizeU, sizeV)
};

struct UIDrawCallData
{
    uint32_t textureIndex;
    uint32_t startVertex;
    uint32_t numVertices;
};

struct CharacterRect
{
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
};

class UIRenderer
{
private:
    friend VulkanRenderer;

    const uint32_t START_NUM_MAX_ELEMENTS = 256;

    std::unordered_map<char, CharacterRect> characterRects;
    uint32_t uiFontTextureIndex;

    std::vector<UIElementData> uiElementData;
    std::vector<UIDrawCallData> uiDrawCallData;

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

    SceneHandler* sceneHandler;

    void prepareForGPU();
    void resetRender();

public:
    UIRenderer();

    void setSceneHandler(SceneHandler* sceneHandler);
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
        const glm::uvec2& tileDimension);

    void setTexture(const uint32_t& textureIndex);
    void renderTexture(
        const glm::vec2& position,
        const glm::vec2& dimension,
        const glm::uvec4 textureCoords = glm::uvec4(0, 0, 1, 1)
    );
    void renderTexture(
        const glm::vec3& worldPosition,
        const glm::vec2& dimension,
        const glm::uvec4 textureCoords = glm::uvec4(0, 0, 1, 1)
    );
    void renderString(
        const std::string& text,
        const glm::vec2& position,
        const glm::vec2& charDimension,
        const float& charMargin = 0.0f,
        const StringAlignment& alignment = StringAlignment::CENTER
    );
    void renderString(
        const std::string& text,
        const glm::vec3& worldPosition,
        const glm::vec2& charDimension,
        const float& charMargin = 0.0f,
        const StringAlignment& alignment = StringAlignment::CENTER
    );

    inline const StorageBufferID& getStorageBufferID() const { return this->storageBufferID; }
    inline std::vector<UIElementData>& getUiElementData() { return this->uiElementData; }
    inline const std::vector<UIDrawCallData>& getUiDrawCallData() const { return this->uiDrawCallData; }
    inline const uint32_t& getUiTextureIndex() const { return this->uiTextureIndex; }
    inline ShaderInput& getShaderInput() { return this->uiShaderInput; }
    inline const Pipeline& getPipeline() const { return this->uiPipeline; }

    // Mostly for Lua use
    inline ResourceManager* getResourceManager() const { return this->resourceManager; }
};