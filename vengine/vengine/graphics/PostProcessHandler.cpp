#include "pch.h"
#include "PostProcessHandler.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/FramebufferArray.hpp"
#include "vulkan/CommandBufferArray.hpp"
#include "Texture.hpp"

void PostProcessHandler::create(const vk::Extent2D& windowExtent)
{
	// Clamp to border sampler
	TextureSettings textureSettings{};
	textureSettings.samplerSettings.addressMode =
		vk::SamplerAddressMode::eClampToBorder;

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
		MAX_NUM_MIP_LEVELS,
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

	// Downsampling image views and extents
	uint32_t currentWidth = windowExtent.width;
	uint32_t currentHeight = windowExtent.height;
	this->mipExtents.resize(MAX_NUM_MIP_LEVELS);
	std::vector<std::vector<vk::ImageView>> framebufferImageViews(MAX_NUM_MIP_LEVELS);
	std::string str;
	for (uint32_t i = 0; i < MAX_NUM_MIP_LEVELS; ++i)
	{
		framebufferImageViews[i] = { this->hdrRenderTexture.getMipImageView(i) };

		// Extents
		this->mipExtents[i].setWidth(currentWidth);
		this->mipExtents[i].setHeight(currentHeight);
		if (currentWidth > 1) currentWidth >>= 1;
		if (currentHeight > 1) currentHeight >>= 1;

		str += "(" + std::to_string(this->mipExtents[i].width) +
			", " + std::to_string(this->mipExtents[i].height) + ") ";
	}
	Log::write(str);

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

	// Mip framebuffers
	this->mipFramebuffers.create(
		*this->device,
		this->downRenderPass,
		this->mipExtents,
		framebufferImageViews
	);
}

PostProcessHandler::PostProcessHandler()
	: physicalDevice(nullptr),
	device(nullptr),
	vma(nullptr),
	transferQueue(nullptr),
	commandPool(nullptr),
	resourceManager(nullptr),
	renderPassBase(nullptr),
	framesInFlight(0),
	currentNumMipLevels(PostProcessHandler::MAX_NUM_MIP_LEVELS)
{}

void PostProcessHandler::init(
	PhysicalDevice& physicalDevice,
	Device& device,
	VmaAllocator& vma,
	RenderPass& renderPassBase,
	vk::Queue& transferQueue,
	vk::CommandPool& commandPool,
	ResourceManager& resourceManager,
	const uint32_t& framesInFlight,
	const vk::Extent2D& windowExtent)
{
	this->physicalDevice = &physicalDevice;
	this->device = &device;
	this->vma = &vma;
	this->renderPassBase = &renderPassBase;
	this->transferQueue = &transferQueue;
	this->commandPool = &commandPool;
	this->resourceManager = &resourceManager;
	this->framesInFlight = framesInFlight;

	// Render passes
	this->downRenderPass.createRenderPassBloomDownsample(
		*this->device
	);
	this->upRenderPass.createRenderPassBloomUpsample(
		*this->device
	);

	// Command buffers
	this->downCommandBuffers.resize(this->framesInFlight);
	this->upCommandBuffers.resize(this->framesInFlight);
	for (uint32_t i = 0; i < this->framesInFlight; ++i)
	{
		this->downCommandBuffers[i].createCommandBuffers(
			*this->device,
			*this->commandPool,
			MAX_NUM_MIP_LEVELS
		);
		this->upCommandBuffers[i].createCommandBuffers(
			*this->device,
			*this->commandPool,
			MAX_NUM_MIP_LEVELS
		);
	}

	// Create resources dependent on the window
	this->create(windowExtent);

	FrequencyInputLayout texInputLayout{};
	texInputLayout.addBinding(vk::DescriptorType::eCombinedImageSampler);

	// Downsample shader input
	this->downShaderInput.beginForInput(
		*this->physicalDevice,
		*this->device,
		*this->vma,
		*this->resourceManager,
		this->framesInFlight
	);

	this->downShaderInput.addPushConstant(
		sizeof(ResolutionPushConstantData),
		vk::ShaderStageFlagBits::eFragment
	);

	// Input
	this->downShaderInput.makeFrequencyInputLayout(texInputLayout);

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

	// Upsample shader input
	this->upShaderInput.beginForInput(
		*this->physicalDevice,
		*this->device,
		*this->vma,
		*this->resourceManager,
		this->framesInFlight
	);

	this->upShaderInput.addPushConstant(
		sizeof(ResolutionPushConstantData),
		vk::ShaderStageFlagBits::eFragment
	);

	// Input
	this->upShaderInput.makeFrequencyInputLayout(texInputLayout);

	this->upShaderInput.endForInput();

	// Upsample pipeline
	this->upPipeline.createPipeline(
		*this->device,
		this->upShaderInput,
		this->upRenderPass,
		VertexStreams{},
		"bloomUpsample.vert.spv",
		"bloomUpsample.frag.spv",
		false
	);

	// Descriptor indices
	this->mipDescriptorIndices.resize(PostProcessHandler::MAX_NUM_MIP_LEVELS);
	for (uint32_t i = 0; i < PostProcessHandler::MAX_NUM_MIP_LEVELS; ++i)
	{
		FrequencyInputBindings inputBinding{};
		inputBinding.texture = &this->hdrRenderTexture;
		inputBinding.imageView = &this->hdrRenderTexture.getMipImageView(i);
		this->mipDescriptorIndices[i] =
			this->downShaderInput.addFrequencyInput({ inputBinding });
		this->upShaderInput.addFrequencyInput({ inputBinding });
	}
}

void PostProcessHandler::recreate(const vk::Extent2D& windowExtent)
{
	// Cleanup old resources
	this->hdrRenderTexture.cleanup();
	this->depthTexture.cleanup();
	this->renderFramebuffer.cleanup();
	this->mipFramebuffers.cleanup();

	this->create(windowExtent);
}

void PostProcessHandler::cleanup()
{
	this->renderFramebuffer.cleanup();
	this->mipFramebuffers.cleanup();

	this->mipDescriptorIndices.clear();

	this->upPipeline.cleanup();
	this->upShaderInput.cleanup();
	this->upCommandBuffers.clear();
	this->upRenderPass.cleanup();

	this->downPipeline.cleanup();
	this->downShaderInput.cleanup();
	this->downCommandBuffers.clear();
	this->downRenderPass.cleanup();

	this->depthTexture.cleanup();
	this->hdrRenderTexture.cleanup();
}

void PostProcessHandler::setCurrentNumMipLevels(const uint32_t& numMipLevels)
{
	this->currentNumMipLevels = std::max(
		std::min(
			numMipLevels, 
			PostProcessHandler::MAX_NUM_MIP_LEVELS), 
		3u
	);
}