#include "UIRenderer.hpp"
#include "../resource_management/ResourceManager.hpp"

UIRenderer::UIRenderer()
    : currentElementIndex(0),
    numRenderVerts(0),
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
    this->uiElementData.clear();
    this->uiElementData.resize(START_NUM_MAX_ELEMENTS);

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

void UIRenderer::setUiTexture(const uint32_t& textureIndex)
{
    this->uiTextureIndex = textureIndex;

    Texture& uiTexture = 
        this->resourceManager->getTexture(this->uiTextureIndex);

    this->uiTextureWidth = static_cast<float>(uiTexture.getWidth());
    this->uiTextureHeight = static_cast<float>(uiTexture.getHeight());
}

void UIRenderer::beginUI()
{
    this->currentElementIndex = 0;
}

void UIRenderer::renderTexture(
    glm::vec4 subTextureRect,
    const float& x, 
    const float& y, 
    const float& width, 
    const float& height)
{
    if (this->currentElementIndex >= START_NUM_MAX_ELEMENTS)
    {
        Log::error("Reached maximum number of ui elements.");
        return;
    }

    // Normalized texture coordinates
    subTextureRect.x /= this->uiTextureWidth;
    subTextureRect.z /= this->uiTextureWidth;
    subTextureRect.y /= this->uiTextureHeight;
    subTextureRect.w /= this->uiTextureHeight;

    // Set element data
    this->uiElementData[this->currentElementIndex] =
    {
        subTextureRect,
        glm::vec4(x, y, width, height)
    };

    // Next ui element
    this->currentElementIndex++;
}

void UIRenderer::endUI()
{
    this->numRenderVerts = this->currentElementIndex * 6;
}
