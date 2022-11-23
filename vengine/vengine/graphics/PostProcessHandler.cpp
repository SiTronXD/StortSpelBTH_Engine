#include "pch.h"
#include "PostProcessHandler.hpp"
#include "../resource_management/ResourceManager.hpp"
#include "vulkan/Device.hpp"
#include "vulkan/FramebufferArray.hpp"
#include "vulkan/CommandBufferArray.hpp"
#include "Texture.hpp"

void PostProcessHandler::updateNumMipLevelsInUse()
{
	float i = std::max((float) this->mipExtents[0].height, 1.0f);

	// What we want:
	//  2k - 1080p - 0 extra mips
	//   ; - 1619p - 0 extra mips
	//   ; - 1620p - 1 extra mips
	//  4k - 2160p - 1 extra mips
	//   ; - 3239p - 1 extra mips
	//   ; - 3240p - 2 extra mips
	//  8k - 4320p - 2 extra mips
	//   ; - 6479p - 2 extra mips
	//   ; - 6480p - 3 extra mips
	// 16k - 8640p - 3 extra mips

	// Avoid negative unsigned int cast
	i = std::max(i, 1080.0f);
	uint32_t extraMipLevels = uint32_t(
		std::log((i * 2.0f / 3.0f) * 2.0f / 1080.0f) / std::log(2.0f)
	);

	// Calculate actual number of mip levels in use
	this->numMipLevelsInUse = std::clamp(
		this->desiredNumMipLevels + extraMipLevels,
		PostProcessHandler::MIN_NUM_MIP_LEVELS,
		PostProcessHandler::MAX_NUM_MIP_LEVELS
	);
}

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
	for (uint32_t i = 0; i < MAX_NUM_MIP_LEVELS; ++i)
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

	// Mip framebuffers
	this->mipFramebuffers.create(
		*this->device,
		this->downRenderPass,
		this->mipExtents,
		framebufferImageViews
	);

	this->updateNumMipLevelsInUse();
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
	desiredNumMipLevels(PostProcessHandler::MAX_NUM_MIP_LEVELS / 2),
	numMipLevelsInUse(PostProcessHandler::MAX_NUM_MIP_LEVELS / 2)
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
		"fullscreenQuad.vert.spv",
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
		"fullscreenQuad.vert.spv",
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

	// Create texture resources
	this->create(windowExtent);

	// Update descriptor sets
	for (uint32_t i = 0; i < this->mipDescriptorIndices.size(); ++i)
	{
		FrequencyInputBindings inputBinding{};
		inputBinding.texture = &this->hdrRenderTexture;
		inputBinding.imageView = &this->hdrRenderTexture.getMipImageView(i);

		this->downShaderInput.updateFrequencyInput({ inputBinding }, this->mipDescriptorIndices[i]);
		this->upShaderInput.updateFrequencyInput({ inputBinding }, this->mipDescriptorIndices[i]);
	}
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

void PostProcessHandler::setDesiredNumMipLevels(const uint32_t& numMipLevels)
{
	this->desiredNumMipLevels = std::clamp(
		numMipLevels,
		PostProcessHandler::MIN_NUM_MIP_LEVELS,
		PostProcessHandler::MAX_NUM_MIP_LEVELS
	);

	this->updateNumMipLevelsInUse();
}