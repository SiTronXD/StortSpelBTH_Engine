#include "pch.h"
#include "PostProcessHandler.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/FramebufferArray.hpp"
#include "vulkan/CommandBufferArray.hpp"
#include "Texture.hpp"

PostProcessHandler::PostProcessHandler()
	: physicalDevice(nullptr),
	device(nullptr),
	vma(nullptr),
	transferQueue(nullptr),
	commandPool(nullptr),
	resourceManager(nullptr),
	renderPassBase(nullptr),
	framesInFlight(0)
{}

void PostProcessHandler::init(
	PhysicalDevice& physicalDevice,
	Device& device,
	VmaAllocator& vma,
	RenderPass& renderPassBase,
	vk::Queue& transferQueue,
	vk::CommandPool& commandPool,
	ResourceManager& resourceManager,
	const uint32_t& framesInFlight)
{
	this->physicalDevice = &physicalDevice;
	this->device = &device;
	this->vma = &vma;
	this->renderPassBase = &renderPassBase;
	this->transferQueue = &transferQueue;
	this->commandPool = &commandPool;
	this->resourceManager = &resourceManager;
	this->framesInFlight = framesInFlight;
}

void PostProcessHandler::create(const vk::Extent2D& windowExtent)
{
	TextureSettings textureSettings{};

	// HDR render texture
	this->hdrRenderTexture.createRenderableTexture(
		*this->physicalDevice,
		*this->device,
		*this->vma,
		*this->transferQueue,
		*this->commandPool,
		PostProcessHandler::HDR_FORMAT,
		windowExtent.width,
		windowExtent.height,
		NUM_MIP_LEVELS,
		this->resourceManager->addSampler(textureSettings),
		vk::ImageUsageFlagBits::eSampled
	);

	// Create depth texture
	this->depthTexture.createAsDepthTexture(
		*this->physicalDevice,
		*this->device,
		*this->vma,
		windowExtent.width,
		windowExtent.height
	);

	// Render pass
	this->downRenderPass.createRenderPassBloom(
		*this->device,
		this->hdrRenderTexture
	);

	// Downsampling image views and extents
	uint32_t currentWidth = windowExtent.width;
	uint32_t currentHeight = windowExtent.height;
	this->mipExtents.resize(NUM_MIP_LEVELS);
	std::vector<std::vector<vk::ImageView>> framebufferImageViews(NUM_MIP_LEVELS);
	for (uint32_t i = 0; i < NUM_MIP_LEVELS; ++i)
	{
		framebufferImageViews[i] = { this->hdrRenderTexture.getMipImageView(i) };

		// Extents
		this->mipExtents[i].setWidth(currentWidth);
		this->mipExtents[i].setHeight(currentHeight);
		if (currentWidth > 1) currentWidth >>= 1;
		if (currentHeight > 1) currentHeight >>= 1;
	}

	// Framebuffer for rendering
	this->renderFramebuffer.create(
		*this->device,
		*this->renderPassBase,
		this->mipExtents[0],
		{
			{
				this->hdrRenderTexture.getImageView(),
				this->depthTexture.getImageView()
			}
		}
	);

	// Downsample framebuffers
	this->downFramebuffers.create(
		*this->device,
		this->downRenderPass,
		this->mipExtents,
		framebufferImageViews
	);

	// Downsample command buffers
	this->downCommandBuffers.resize(this->framesInFlight);
	for (uint32_t i = 0; i < this->framesInFlight; ++i)
	{
		this->downCommandBuffers[i].createCommandBuffers(
			*this->device,
			*this->commandPool,
			NUM_MIP_LEVELS
		);
	}

	// Downsample shader input
	this->downShaderInput.beginForInput(
		*this->physicalDevice,
		*this->device,
		*this->vma,
		*this->resourceManager,
		this->framesInFlight
	);

	// Input
	FrequencyInputLayout downInputLayout{};
	downInputLayout.addBinding(vk::DescriptorType::eCombinedImageSampler);
	this->downShaderInput.makeFrequencyInputLayout(downInputLayout);

	this->downShaderInput.endForInput();

	// Downsample pipeline
	this->downPipeline.createPipeline(
		*this->device, 
		this->downShaderInput, 
		this->downRenderPass,
		VertexStreams{},
		"bloomDownsample.vert.spv",
		"bloomDownsample.frag.spv",
		false
	);

	// Descriptor indices
	this->mipDescriptorIndices.resize(PostProcessHandler::NUM_MIP_LEVELS);
	for (uint32_t i = 0; i < PostProcessHandler::NUM_MIP_LEVELS; ++i)
	{
		FrequencyInputBindings inputBinding{};
		inputBinding.texture = &this->hdrRenderTexture;
		inputBinding.imageView = &this->hdrRenderTexture.getMipImageView(i);
		this->mipDescriptorIndices[i] = 
			this->downShaderInput.addFrequencyInput({ inputBinding });
	}
}

void PostProcessHandler::cleanup()
{
	this->renderFramebuffer.cleanup();

	this->mipDescriptorIndices.clear();

	this->downPipeline.cleanup();
	this->downShaderInput.cleanup();
	this->downCommandBuffers.clear();
	this->downFramebuffers.cleanup();
	this->downRenderPass.cleanup();

	this->depthTexture.cleanup();
	this->hdrRenderTexture.cleanup();
}