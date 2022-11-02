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
    const std::vector<std::vector<Pixel>>& pixels =
        this->resourceManager->getTexture(bitmapFontTextureIndex).getCpuPixels();

    this->characterTileWidth = static_cast<float>(tileWidth);
    this->characterTileHeight = static_cast<float>(tileHeight);

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
                        if (pixels[tempCharX][tempCharY].a > 0)
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
            charRect.x = (float) charX;
            charRect.y = (float) charY;
            charRect.width = (float) charWidth;
            charRect.height = (float) charHeight;

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
    const float& x, 
    const float& y, 
    const float& width, 
    const float& height,
    const float& u0,
    const float& v0,
    const float& u1,
    const float& v1)
{
    if (this->currentElementIndex >= START_NUM_MAX_ELEMENTS)
    {
        Log::error("Reached maximum number of rendered ui elements.");
        return;
    }

    // Set element data
    this->uiElementData[this->currentElementIndex].transform =
        ResTranslator::transformRect(x, y, width, height);
    this->uiElementData[this->currentElementIndex].uvRect =
        glm::vec4(u0, v0, u1, v1);

    // Next ui element
    this->currentElementIndex++;
}

void UIRenderer::renderString(
    const std::string& text,
    const float& x,
    const float& y,
    const float& characterWidth,
    const float& characterHeight)
{
    float currentCharWidth = 0.0f;
    float currentCharHeight = 0.0f;

    // Start at left edge of the first character
    float currentSizeX = 
        -characterWidth * 
        (this->characterRects[text[0]].width / this->characterTileWidth) * 
        0.5f;

    // Render character by character
    for (size_t i = 0; i < text.length(); ++i)
    {
        // Find rectangle from map
        const CharacterRect& charRect = this->characterRects[text[i]];

        // Current character size
        currentCharWidth =
            characterWidth * (charRect.width / this->characterTileWidth);
        currentCharHeight =
            characterHeight * (charRect.height / this->characterTileHeight);

        // Render character as texture
        this->renderTexture(
            x + currentSizeX + currentCharWidth * 0.5f,
            y, 
            currentCharWidth,
            currentCharHeight,
            charRect.x, 
            charRect.y, 
            charRect.width, 
            charRect.height 
        );

        currentSizeX += currentCharWidth;
    }
}