#include "pch.h"
#include "UIRenderer.hpp"
#include "ResTranslator.hpp"
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
    this->uiDrawCallData.clear();
}

UIRenderer::UIRenderer()
    : currentElementIndex(0),
    uiTextureIndex(~0u),
    uiTextureWidth(0),
    uiTextureHeight(0),
    uiSamplerID(~0u),
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
	vk::RenderPass& renderPass,
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

    // UI renderer shader input
    this->uiShaderInput.beginForInput(
        *this->physicalDevice,
        *this->device,
        *this->vma,
        *this->resourceManager,
        this->framesInFlight);
    this->uiSamplerID = this->uiShaderInput.addSampler();
    this->uiShaderInput.setNumShaderStorageBuffers(1);
    this->storageBufferID = this->uiShaderInput.addStorageBuffer(
        this->uiElementData.size() *
        sizeof(this->uiElementData[0])
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
    const uint32_t& tileWidth,
    const uint32_t& tileHeight)
{
    const Texture& fontTexture = 
        this->resourceManager->getTexture(bitmapFontTextureIndex);

    this->oneOverCharTileWidth = 1.0f / static_cast<float>(tileWidth);
    this->oneOverCharTileHeight = 1.0f / static_cast<float>(tileHeight);

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

            uint32_t charX = x * tileWidth;
            uint32_t charY = y * tileHeight;
            uint32_t charWidth = tileWidth;
            uint32_t charHeight = tileHeight;

            // Make rect smaller, but not for spaces
            if (currentChar != ' ')
            {
                uint32_t minX = charX + charWidth;
                uint32_t minY = charY + charHeight;
                uint32_t maxX = charX;
                uint32_t maxY = charY;

                // Loop through pixels inside char rect
                for (uint32_t tempCharY = charY; tempCharY < charY + tileHeight; ++tempCharY)
                {
                    for (uint32_t tempCharX = charX; tempCharX < charX + tileWidth; ++tempCharX)
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
}

void UIRenderer::setTexture(const uint32_t& textureIndex)
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
}

void UIRenderer::renderTexture(
    const glm::vec2& position,
    const glm::vec2& dimension,
    const glm::uvec4 textureCoords)
{
    if (this->currentElementIndex >= START_NUM_MAX_ELEMENTS)
    {
        Log::error("Reached maximum number of rendered ui elements.");
        return;
    }

    // Set element data
    this->uiElementData[this->currentElementIndex].transform =
        ResTranslator::transformRect(position, dimension);
    this->uiElementData[this->currentElementIndex].uvRect =
        textureCoords;

    // Next ui element
    this->currentElementIndex++;
}

void UIRenderer::renderTexture(
    const glm::vec3& position,
    const glm::vec2& dimension,
    const glm::uvec4 textureCoords)
{
    Scene* scene = this->sceneHandler->getScene();
    if (!scene->entityValid(scene->getMainCameraID())) { return; }

    glm::vec4 pos = scene->getMainCamera()->projection * scene->getMainCamera()->view * glm::vec4(position, 1.0f);
    glm::vec2 orig = glm::vec2(pos);
    if (pos.z > pos.w || pos.z <= 0) { return; }

    pos.x = (pos.x / pos.w) * (ResTranslator::INTERNAL_WIDTH >> 1);
    pos.y = (pos.y / pos.w) * (ResTranslator::INTERNAL_HEIGHT >> 1);
    pos.z /= pos.w;

    Transform& camTransform = scene->getComponent<Transform>(scene->getMainCameraID());
    float dot = glm::dot(glm::normalize(glm::vec3(pos) - camTransform.position), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec2 size = 10.0f * dimension / (glm::length(glm::vec3(pos) - camTransform.position) * dot);
    
    Log::write("(" + std::to_string(size.x) + ", " + std::to_string(size.y) + ")");
    this->renderTexture(glm::vec2(pos), size, textureCoords);
}

void UIRenderer::renderString(
    const std::string& text,
    const glm::vec2& position,
    const glm::vec2& charDimension,
    const float& charMargin,
    const StringAlignment& alignment)
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

    // Render character by character
    for (size_t i = 0; i < text.length(); ++i)
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
            glm::uvec4(charRect->x, charRect->y, charRect->width, charRect->height)
        );

        currentSizeX += charMargin + currentCharWidth;
    }
}