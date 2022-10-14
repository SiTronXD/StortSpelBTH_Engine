#include "UIRenderer.hpp"
#include "ResTranslator.hpp"
#include "../resource_management/ResourceManager.hpp"

UIRenderer::UIRenderer()
    : currentElementIndex(0),
    uiTextureIndex(~0u),
    uiTextureWidth(0),
    uiTextureHeight(0),
    uiSamplerID(~0u),
    storageBufferID(~0u),
    device(nullptr),
    vma(nullptr),
    resourceManager(nullptr)
{
}

void UIRenderer::create(
	PhysicalDevice& physicalDevice,
	Device& device,
	VmaAllocator& vma,
	ResourceManager& resourceManager,
	vk::RenderPass& renderPass,
    vk::Queue& transferQueue,
    vk::CommandPool& transferCommandPool,
	const uint32_t& framesInFlight)
{
    this->device = &device;
    this->vma = &vma;
    this->resourceManager = &resourceManager;

	// Target data from the vertex buffers
    this->uiElementData.resize(START_NUM_MAX_ELEMENTS);
    this->uiDrawCallData.reserve(START_NUM_MAX_ELEMENTS);

	// Shader input, with no inputs for now
	this->uiShaderInput.beginForInput(
		physicalDevice,
		device,
		vma,
		resourceManager,
		framesInFlight);
    this->uiSamplerID = this->uiShaderInput.addSampler();
    this->uiShaderInput.setNumShaderStorageBuffers(1);
    this->storageBufferID = this->uiShaderInput.addStorageBuffer(
        this->uiElementData.size() *
            sizeof(this->uiElementData[0])
    );
	this->uiShaderInput.endForInput();

	// Pipeline
	this->uiPipeline.createPipeline(
		device,
		this->uiShaderInput,
		renderPass,
        VertexStreams{},
		"ui.vert.spv",
		"ui.frag.spv",
        false
	);
}

void UIRenderer::cleanup()
{
    for (size_t i = 0; i < this->vertexBuffers.size(); ++i)
    {
        this->device->getVkDevice().destroyBuffer(this->vertexBuffers[i]);
        vmaFreeMemory(*this->vma, this->vertexBufferMemories[i]);
    }

	this->uiPipeline.cleanup();
	this->uiShaderInput.cleanup();
}

void UIRenderer::beginUI()
{
    this->currentElementIndex = 0;
    this->uiDrawCallData.clear();
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
    const float& height)
{
    if (this->currentElementIndex >= START_NUM_MAX_ELEMENTS)
    {
        Log::error("Reached maximum number of rendered ui elements.");
        return;
    }

    // Set element data
    this->uiElementData[this->currentElementIndex].transform =
        ResTranslator::transformRect(x, y, width, height);

    // Next ui element
    this->currentElementIndex++;
}

void UIRenderer::endUI()
{
    // Num vertices for last draw call
    this->uiDrawCallData[this->uiDrawCallData.size() - 1].numVertices =
        this->currentElementIndex * 6 - this->uiDrawCallData[this->uiDrawCallData.size() - 1].startVertex;
}
