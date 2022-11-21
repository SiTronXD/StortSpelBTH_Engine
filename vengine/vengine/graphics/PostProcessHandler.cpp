#include "pch.h"
#include "PostProcessHandler.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/FramebufferArray.hpp"
#include "vulkan/CommandBufferArray.hpp"
#include "Texture.hpp"

#include "stb_image.h"
void PostProcessHandler::init(
	PhysicalDevice& physicalDevice,
	Device& device,
	VmaAllocator& vma,
	vk::Queue& transferQueue,
	vk::CommandPool& commandPool,
	ResourceManager& resourceManager)
{
	
	uint32_t width = 1280;
	uint32_t height = 720;

	TextureSettings textureSettings{};

	// Emission texture
	this->hdrRenderTexture.createRenderableTexture(
		physicalDevice,
		device,
		vma,
		transferQueue,
		commandPool,
		vk::Format::eR8G8B8A8Unorm,
		uint32_t(width),
		uint32_t(height),
		NUM_MIP_LEVELS
	);

	// Render pass
	this->renderPass.createRenderPassBloom(
		device, 
		this->hdrRenderTexture
	);

	// Framebuffers
	uint32_t currentWidth = width;
	uint32_t currentHeight = height;
	this->extents.resize(NUM_MIP_LEVELS);
	std::vector<std::vector<vk::ImageView>> framebufferImageViews(NUM_MIP_LEVELS);
	for (uint32_t i = 0; i < NUM_MIP_LEVELS; ++i)
	{
		framebufferImageViews[i] = { this->hdrRenderTexture.getMipImageView(i) };

		// Extents
		this->extents[i].setWidth(currentWidth);
		this->extents[i].setHeight(currentHeight);
		if (currentWidth > 1) currentWidth >>= 1;
		if (currentHeight > 1) currentHeight >>= 1;
	}
	this->framebuffers.create(
		device, 
		this->renderPass,
		this->extents,
		framebufferImageViews
	);

	// Command buffers
	this->commandBuffers.createCommandBuffers(
		device,
		commandPool,
		NUM_MIP_LEVELS
	);
}

void PostProcessHandler::initForScene()
{

}

void PostProcessHandler::cleanup()
{
	this->framebuffers.cleanup();
	this->renderPass.cleanup();
	this->hdrRenderTexture.cleanup();
}