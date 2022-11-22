#pragma once

#include <stdint.h>
#include "Texture.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/FramebufferArray.hpp"
#include "vulkan/CommandBufferArray.hpp"
#include "vulkan/Pipeline.hpp"

class Texture;
class RenderPass;
class FramebufferArray;
class CommandBufferArray;
class ResourceManager;
class ShaderInput;
class Pipeline;

class PostProcessHandler
{
private:
	Texture hdrRenderTexture;
	Texture depthTexture;
	std::vector<uint32_t> mipDescriptorIndices;

	// Render to HDR texture
	FramebufferArray renderFramebuffer;

	// Downsample
	RenderPass downRenderPass;
	FramebufferArray downFramebuffers;
	std::vector<CommandBufferArray> downCommandBuffers; // downCommandBuffers[currentFrame][mipLevel]
	ShaderInput downShaderInput;
	Pipeline downPipeline;

	std::vector<vk::Extent2D> mipExtents;

	PhysicalDevice* physicalDevice;
	Device* device;
	VmaAllocator* vma;
	RenderPass* renderPassBase;
	vk::Queue* transferQueue;
	vk::CommandPool* commandPool;
	ResourceManager* resourceManager;

	uint32_t framesInFlight;

public:
	inline static const uint32_t NUM_MIP_LEVELS = 8;
	inline static const vk::Format HDR_FORMAT =
		vk::Format::eR16G16B16A16Sfloat;

	PostProcessHandler();

	void init(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		RenderPass& baseRenderPass,
		vk::Queue& transferQueue,
		vk::CommandPool& commandPool,
		ResourceManager& resourceManager,
		const uint32_t& framesInFlight);
	void create(const vk::Extent2D& windowExtent);

	void cleanup();

	inline Texture& getHdrRenderTexture() { return this->hdrRenderTexture; }
	inline CommandBuffer& getDownsampleCommandBuffer(const uint32_t& currentFrame, const uint32_t& mipLevel) { return this->downCommandBuffers[currentFrame][mipLevel]; }
	inline ShaderInput& getDownsampleShaderInput() { return this->downShaderInput; }
	inline const Texture& getDepthTexture() const { return this->depthTexture; }
	inline const RenderPass& getDownsampleRenderPass() const { return this->downRenderPass; }
	inline const Pipeline& getDownsamplePipeline() const { return this->downPipeline; }
	inline const vk::Framebuffer& getRenderVkFramebuffer() const { return this->renderFramebuffer[0]; }
	inline const vk::Framebuffer& getDownsampleVkFramebuffer(const uint32_t& mipLevel) const { return this->downFramebuffers[mipLevel]; }
	inline const vk::Extent2D& getMipExtent(const uint32_t& mipLevel) const { return this->mipExtents[mipLevel]; }
	inline const uint32_t& getMipDescriptorIndex(const uint32_t& mipLevel) const { return this->mipDescriptorIndices[mipLevel]; }
};