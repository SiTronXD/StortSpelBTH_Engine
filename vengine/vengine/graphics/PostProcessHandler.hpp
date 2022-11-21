#pragma once

#include <stdint.h>
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
	RenderPass renderPass;
	FramebufferArray framebuffers;
	CommandBufferArray commandBuffers;
	std::vector<vk::Extent2D> extents;

public:
	static const uint32_t NUM_MIP_LEVELS = 8;

	void init(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		vk::Queue& transferQueue,
		vk::CommandPool& commandPool,
		ResourceManager& resourceManager);
	void initForScene();

	void cleanup();

	inline const Texture& getHdrRenderTexture() const { return this->hdrRenderTexture; }
};