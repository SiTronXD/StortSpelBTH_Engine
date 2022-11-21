#pragma once

#include <stdint.h>
#include "Texture.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/FramebufferArray.hpp"
#include "vulkan/CommandBufferArray.hpp"

class Texture;
class RenderPass;
class FramebufferArray;
class CommandBufferArray;
class ResourceManager;

class PostProcessHandler
{
private:
	Texture hdrRenderTexture;
	Texture depthTexture;

	// Render to HDR texture
	FramebufferArray renderFramebuffer;

	// Downsample
	RenderPass renderPass;
	FramebufferArray framebuffers;
	CommandBufferArray commandBuffers;
	std::vector<vk::Extent2D> extents;

	PhysicalDevice* physicalDevice;
	Device* device;
	VmaAllocator* vma;
	RenderPass* renderPassBase;
	vk::Queue* transferQueue;
	vk::CommandPool* commandPool;
	ResourceManager* resourceManager;

public:
	static const uint32_t NUM_MIP_LEVELS = 8;
	static const vk::Format HDR_FORMAT = vk::Format::eR16G16B16A16Sfloat;

	PostProcessHandler();

	void init(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		RenderPass& baseRenderPass,
		vk::Queue& transferQueue,
		vk::CommandPool& commandPool,
		ResourceManager& resourceManager);
	void create(const vk::Extent2D& windowExtent);

	void cleanup();

	inline Texture& getHdrRenderTexture() { return this->hdrRenderTexture; }
	inline const Texture& getDepthTexture() const { return this->depthTexture; }
	inline const vk::Framebuffer& getRenderVkFramebuffer() const { return this->renderFramebuffer[0]; }
};