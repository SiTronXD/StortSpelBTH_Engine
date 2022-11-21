#include "pch.h"
#include "PostProcessHandler.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/FramebufferArray.hpp"
#include "vulkan/CommandBufferArray.hpp"
#include "Texture.hpp"

PostProcessHandler::PostProcessHandler()
	: physicalDevice(nullptr),
	device(nullptr),
	vma(nullptr),
	transferQueue(nullptr),
	commandPool(nullptr),
	resourceManager(nullptr)
{}

void PostProcessHandler::init(
	PhysicalDevice& physicalDevice,
	Device& device,
	VmaAllocator& vma,
	vk::Queue& transferQueue,
	vk::CommandPool& commandPool,
	ResourceManager& resourceManager)
{
	this->physicalDevice = &physicalDevice;
	this->device = &device;
	this->vma = &vma;
	this->transferQueue = &transferQueue;
	this->commandPool = &commandPool;
	this->resourceManager = &resourceManager;
}

void PostProcessHandler::create(const vk::Extent2D& windowExtent)
{
	TextureSettings textureSettings{};

	// Emission texture
	this->hdrRenderTexture.createRenderableTexture(
		*this->physicalDevice,
		*this->device,
		*this->vma,
		*this->transferQueue,
		*this->commandPool,
		vk::Format::eR8G8B8A8Unorm,
		windowExtent.width,
		windowExtent.height,
		NUM_MIP_LEVELS
	);

	// Render pass
	this->renderPass.createRenderPassBloom(
		*this->device,
		this->hdrRenderTexture
	);

	// Framebuffers
	uint32_t currentWidth = windowExtent.width;
	uint32_t currentHeight = windowExtent.height;
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
		*this->device,
		this->renderPass,
		this->extents,
		framebufferImageViews
	);

	// Command buffers
	this->commandBuffers.createCommandBuffers(
		*this->device,
		*this->commandPool,
		NUM_MIP_LEVELS
	);

	// Create depth texture
	this->depthTexture.createAsDepthTexture(
		*this->physicalDevice,
		*this->device,
		*this->vma,
		this->extents[0].width,
		this->extents[0].height
	);
}

void PostProcessHandler::cleanup()
{
	this->framebuffers.cleanup();
	this->renderPass.cleanup();
	this->depthTexture.cleanup();
	this->hdrRenderTexture.cleanup();
}