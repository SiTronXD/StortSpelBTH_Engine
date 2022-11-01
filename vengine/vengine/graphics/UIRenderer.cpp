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
    // Render character by character
    for (size_t i = 0; i < text.length(); ++i)
    {
        this->renderTexture(
            x + i * characterWidth,
            y, 
            characterWidth, 
            characterHeight,
            0, 0, 16, 16 
        );
    }
}