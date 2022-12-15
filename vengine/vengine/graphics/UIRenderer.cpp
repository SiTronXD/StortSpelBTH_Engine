#include "pch.h"
#include "UIRenderer.hpp"
#include "ResTranslator.hpp"
#include "vulkan/RenderPass.hpp"
#include "../resource_management/ResourceManager.hpp"

void UIRenderer::prepareForGPU()
{
    // Num vertices for last draw call
    if (this->uiDrawCallData.size() > 0)
    {
        this->uiDrawCallData[this->uiDrawCallData.size() - 1].numVertices =
            this->currentElementIndex * 6 - this->uiDrawCallData[this->uiDrawCallData.size() - 1].startVertex;
    }
}

void UIRenderer::resetRender()
{
    this->currentElementIndex = 0;
    this->uiTextureIndex = ~0u;
    this->uiDrawCallData.clear();
}

UIRenderer::UIRenderer()
    : currentElementIndex(0),
    uiFontTextureIndex(~0u),
    uiTextureIndex(~0u),
    uiTextureWidth(0),
    uiTextureHeight(0),
    storageBufferID(~0u),
    physicalDevice(nullptr),
    device(nullptr),
    vma(nullptr),
    resourceManager(nullptr),
    renderPass(nullptr),
    framesInFlight(0)
{
}

void UIRenderer::setSceneHandler(SceneHandler* sceneHandler)
{
    this->sceneHandler = sceneHandler;
}

void UIRenderer::create(
	PhysicalDevice& physicalDevice,
	Device& device,
	VmaAllocator& vma,
	ResourceManager& resourceManager,
	RenderPass& renderPass,
	const uint32_t& framesInFlight)
{
    this->physicalDevice = &physicalDevice;
    this->device = &device;
    this->vma = &vma;
    this->resourceManager = &resourceManager;
    this->renderPass = &renderPass;
    this->framesInFlight = framesInFlight;

	// Target data from the vertex buffers
    this->uiElementData.resize(START_NUM_MAX_ELEMENTS);
    this->uiDrawCallData.reserve(START_NUM_MAX_ELEMENTS);
}

void UIRenderer::initForScene()
{
    // Cleanup ui shader input if possible
    this->cleanup();

    // Per draw bindings
    FrequencyInputLayout perDrawInputLayout{};
    perDrawInputLayout.addBinding(vk::DescriptorType::eCombinedImageSampler);

    // UI renderer shader input
    this->uiShaderInput.beginForInput(
        *this->physicalDevice,
        *this->device,
        *this->vma,
        *this->resourceManager,
        this->framesInFlight);
    this->uiShaderInput.setNumShaderStorageBuffers(1);
    this->storageBufferID = this->uiShaderInput.addStorageBuffer(
        this->uiElementData.size() *
        sizeof(this->uiElementData[0]),
        vk::ShaderStageFlagBits::eVertex,
        DescriptorFrequency::PER_MESH // TODO: change this
    );
    this->uiShaderInput.makeFrequencyInputLayout(
        //DescriptorFrequency::PER_DRAW_CALL,
        perDrawInputLayout
    );
    this->uiShaderInput.endForInput();

    // Pipeline
    this->uiPipeline.createPipeline(
        *this->device,
        this->uiShaderInput,
        *this->renderPass,
        VertexStreams{},
        "ui.vert.spv",
        "ui.frag.spv",
        false,
        false,
        false
    );
}

void UIRenderer::cleanup()
{
	this->uiPipeline.cleanup();
	this->uiShaderInput.cleanup();
}

void UIRenderer::setBitmapFont(
    const std::vector<std::string>& characters,
    const uint32_t& bitmapFontTextureIndex,
    const glm::uvec2& tileDimension)
{
    const Texture& fontTexture = 
        this->resourceManager->getTexture(bitmapFontTextureIndex);

    this->oneOverCharTileWidth = 1.0f / static_cast<float>(tileDimension.x);
    this->oneOverCharTileHeight = 1.0f / static_cast<float>(tileDimension.y);

    // Go through rows
    for (uint32_t y = 0, rows = (uint32_t) characters.size(); 
        y < rows; 
        ++y)
    {
        // Go through columns
        for (uint32_t x = 0, cols = (uint32_t) characters[y].length(); 
            x < cols; 
            ++x)
        {
            const char& currentChar = characters[y][x];

            uint32_t charX = x * tileDimension.x;
            uint32_t charY = y * tileDimension.y;
            uint32_t charWidth = tileDimension.x;
            uint32_t charHeight = tileDimension.y;

            // Make rect smaller, but not for spaces
            if (currentChar != ' ')
            {
                uint32_t minX = charX + charWidth;
                uint32_t minY = charY + charHeight;
                uint32_t maxX = charX;
                uint32_t maxY = charY;

                // Loop through pixels inside char rect
                for (uint32_t tempCharY = charY; tempCharY < charY + tileDimension.y; ++tempCharY)
                {
                    for (uint32_t tempCharX = charX; tempCharX < charX + tileDimension.x; ++tempCharX)
                    {
                        // Found pixel
                        if (fontTexture.getCpuPixel(tempCharX, tempCharY).a > 0)
                        {
                            minX = std::min(minX, tempCharX);
                            minY = std::min(minY, tempCharY);
                            maxX = std::max(maxX, tempCharX + 1);
                            maxY = std::max(maxY, tempCharY + 1);
                        }
                    }
                }

                // Apply new size
                charX = minX;
                charY = minY;
                charWidth = maxX - minX;
                charHeight = maxY - minY;
            }

            // Create rect
            CharacterRect charRect{};
            charRect.x = charX;
            charRect.y = charY;
            charRect.width = charWidth;
            charRect.height = charHeight;

            // Add rect
            this->characterRects.insert({ currentChar, charRect });
        }
    }

    this->uiFontTextureIndex = bitmapFontTextureIndex;
}

void UIRenderer::setTexture(const uint32_t& textureIndex)
{
    if (textureIndex != this->uiTextureIndex)
    {
        // Add data for unique draw call
        UIDrawCallData drawCallData{};
        drawCallData.textureIndex = textureIndex;
        drawCallData.startVertex = this->currentElementIndex * 6;

        // Set number of vertices for previous draw call
        if (this->uiDrawCallData.size() > 0)
        {
            // Num vertices for last draw call
            this->uiDrawCallData[this->uiDrawCallData.size() - 1].numVertices =
                this->currentElementIndex * 6 - this->uiDrawCallData[this->uiDrawCallData.size() - 1].startVertex;
        }

        // Set draw call
        this->uiDrawCallData.push_back(drawCallData);
        this->uiTextureIndex = textureIndex;
    }
}

void UIRenderer::renderTexture(
    const glm::vec2& position,
    const glm::vec2& dimension,
    const glm::uvec4 textureCoords,
    const glm::vec4 multiplyColor)
{
    if (this->currentElementIndex >= START_NUM_MAX_ELEMENTS)
    {
        Log::error("Reached maximum number of rendered ui elements.");
        return;
    }

    glm::vec2 extent = dimension / 2.0f;
    glm::vec2 resTranslatorExtent = ResTranslator::getInternalDimensions() * 0.5f;
    if (position.x - extent.x <= resTranslatorExtent.x && position.x + extent.x >= -resTranslatorExtent.x &&
        position.y - extent.y <= resTranslatorExtent.y && position.y + extent.y >= -resTranslatorExtent.y) // Within UI window
    {
        // Set element data
        this->uiElementData[this->currentElementIndex].transform =
            ResTranslator::transformRect(position, dimension);
        this->uiElementData[this->currentElementIndex].multiplyColor = multiplyColor;
        this->uiElementData[this->currentElementIndex].uvRect =
            textureCoords;

        // Next ui element
        this->currentElementIndex++;
    }
}

void UIRenderer::renderTexture(
    const glm::vec3& worldPosition,
    const glm::vec2& dimension,
    const glm::uvec4 textureCoords,
    const glm::vec4 multiplyColor)
{
    Scene* scene = this->sceneHandler->getScene();
    if (!scene->entityValid(scene->getMainCameraID())) { return; }

    Camera* cam = scene->getMainCamera();
    Transform& camTransform = scene->getComponent<Transform>(scene->getMainCameraID());
    cam->updateMatrices(camTransform);

    glm::vec4 pos = cam->viewAndProj * glm::vec4(worldPosition, 1.0f);
    if (pos.z > pos.w || pos.z < 0) { return; }

    glm::vec2 screenPos = (glm::vec2(pos) / pos.w) * ResTranslator::getInternalDimensions() / 2.0f;
    glm::vec2 size = 5.0f * dimension / glm::dot(worldPosition - camTransform.position, camTransform.forward());

    this->renderTexture(screenPos, size, textureCoords, multiplyColor);
}

void UIRenderer::renderTexture(
    const glm::vec3& worldPosition,
    const glm::vec2& dimension,
    glm::vec4& resultingRect,
    const glm::uvec4 textureCoords,
    const glm::vec4 multiplyColor)
{
    Scene* scene = this->sceneHandler->getScene();
    if (!scene->entityValid(scene->getMainCameraID())) { return; }

    Camera* cam = scene->getMainCamera();
    Transform& camTransform = scene->getComponent<Transform>(scene->getMainCameraID());
    cam->updateMatrices(camTransform);

    glm::vec4 pos = cam->viewAndProj * glm::vec4(worldPosition, 1.0f);
    if (pos.z > pos.w || pos.z < 0) { return; }

    glm::vec2 screenPos = (glm::vec2(pos) / pos.w) * ResTranslator::getInternalDimensions() / 2.0f;
    glm::vec2 size = 5.0f * dimension / glm::dot(worldPosition - camTransform.position, camTransform.forward());

    resultingRect.x = screenPos.x;
    resultingRect.y = screenPos.y;
    resultingRect.z = size.x;
    resultingRect.w = size.y;

    this->renderTexture(screenPos, size, textureCoords, multiplyColor);
}

void UIRenderer::renderString(
    const std::string& text,
    const glm::vec2& position,
    const glm::vec2& charDimension,
    const float& charMargin,
    const StringAlignment& alignment,
    const glm::vec4 multiplyColor)
{
    float currentCharWidth = 0.0f;
    float currentCharHeight = 0.0f;

    CharacterRect* charRect = nullptr;

    const float firstCharWidth = 
        charDimension.x *
        (this->characterRects[text[0]].width * this->oneOverCharTileWidth);
    
        // Start at left edge of the first character
    float currentSizeX = -firstCharWidth * 0.5f;

    // Get entire string width after the first character
    // (because it is offset by currentSizeX)
    float stringWidth = 0.0f;
    for (size_t i = 1; i < text.length(); ++i)
    {
        // Find rectangle from map
        charRect = &this->characterRects[text[i]];

        // Current character size
        currentCharWidth =
            charDimension.x * (charRect->width * this->oneOverCharTileWidth);
        
        stringWidth += charMargin + currentCharWidth;
    }

    // Alignment offset
    float alignmentOffset = 
        -((int) alignment) * (stringWidth + firstCharWidth) * 0.5f;

    // Set texture to font texture
    this->setTexture(this->uiFontTextureIndex);

    // Render character by character
    for (size_t i = 0, textLength = text.length(); i < textLength; ++i)
    {
        // Find rectangle from map
        charRect = &this->characterRects[text[i]];

        // Current character size
        currentCharWidth =
            charDimension.x * (charRect->width * this->oneOverCharTileWidth);
        currentCharHeight =
            charDimension.y * (charRect->height * this->oneOverCharTileWidth);

        // Render character as texture
        this->renderTexture(
            glm::vec2(
                position.x + currentSizeX + currentCharWidth * 0.5f - stringWidth * 0.5f + alignmentOffset, 
                position.y - charDimension.y * 0.5f + currentCharHeight * 0.5f),
            glm::vec2(currentCharWidth, currentCharHeight),
            glm::uvec4(charRect->x, charRect->y, charRect->width, charRect->height),
            multiplyColor
        );

        currentSizeX += charMargin + currentCharWidth;
    }
}

void UIRenderer::renderString(
    const std::string& text,
    const glm::vec3& worldPosition,
    const glm::vec2& charDimension,
    const float& charMargin,
    const StringAlignment& alignment,
    const glm::vec4 multiplyColor)
{
    Scene* scene = this->sceneHandler->getScene();
    if (!scene->entityValid(scene->getMainCameraID())) { return; }

    Camera* cam = scene->getMainCamera();
    Transform& camTransform = scene->getComponent<Transform>(scene->getMainCameraID());
    cam->updateMatrices(camTransform);

    glm::vec4 pos = cam->viewAndProj * glm::vec4(worldPosition, 1.0f);
    if (pos.z > pos.w || pos.z < 0) { return; }

    glm::vec2 screenPos = (glm::vec2(pos) / pos.w) * ResTranslator::getInternalDimensions() / 2.0f;
    float dot = 5.0f / glm::dot(worldPosition - camTransform.position, camTransform.forward());
    glm::vec2 size = charDimension * dot;
    float margin = charMargin * dot;

    this->renderString(text, screenPos, size, margin, alignment, multiplyColor);
}
