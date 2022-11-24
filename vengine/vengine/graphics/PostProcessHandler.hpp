#pragma once

#include <stdint.h>
#include "Texture.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/FramebufferArray.hpp"
#include "vulkan/CommandBufferArray.hpp"
#include "vulkan/Pipeline.hpp"
#include "vulkan/ShaderStructs.hpp"

class Texture;
class RenderPass;
class FramebufferArray;
class CommandBufferArray;
class ResourceManager;
class ShaderInput;
class Pipeline;

class PostProcessHandler
{
public:
	inline static const uint32_t MIN_NUM_MIP_LEVELS = 3;
	inline static const uint32_t MAX_NUM_MIP_LEVELS = 10;
	inline static const vk::Format HDR_FORMAT =
		vk::Format::eR16G16B16A16Sfloat;

private:
	Texture hdrRenderTexture;
	Texture depthTexture;
	std::vector<uint32_t> mipDescriptorIndices;
	FramebufferArray mipFramebuffers;

	// Render to HDR texture
	FramebufferArray renderFramebuffer;

	// Downsample
	RenderPass downRenderPass;
	std::vector<CommandBufferArray> downCommandBuffers; // downCommandBuffers[currentFrame][mipLevel]
	ShaderInput downShaderInput;
	Pipeline downPipeline;

	// Upsample
	RenderPass upRenderPass;
	std::vector<CommandBufferArray> upCommandBuffers; // upCommandBuffers[currentFrame][mipLevel]
	ShaderInput upShaderInput;
	Pipeline upPipeline;

	ResolutionPushConstantData resolutionData{};

	std::vector<vk::Extent2D> mipExtents;

	PhysicalDevice* physicalDevice;
	Device* device;
	VmaAllocator* vma;
	RenderPass* renderPassBase;
	vk::Queue* transferQueue;
	vk::CommandPool* commandPool;
	ResourceManager* resourceManager;

	uint32_t framesInFlight;
	uint32_t desiredNumMipLevels;
	uint32_t numMipLevelsInUse;

	void updateNumMipLevelsInUse();
	void create(const vk::Extent2D& windowExtent);

public:
	PostProcessHandler();

	void init(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		RenderPass& baseRenderPass,
		vk::Queue& transferQueue,
		vk::CommandPool& commandPool,
		ResourceManager& resourceManager,
		const uint32_t& framesInFlight,
		const vk::Extent2D& windowExtent);
	void recreate(const vk::Extent2D& windowExtent);

	void cleanup();

	void setDesiredNumMipLevels(const uint32_t& numMipLevels);

	inline Texture& getHdrRenderTexture() { return this->hdrRenderTexture; }
	inline CommandBuffer& getDownsampleCommandBuffer(const uint32_t& currentFrame, const uint32_t& mipLevel) { return this->downCommandBuffers[currentFrame][mipLevel]; }
	inline CommandBuffer& getUpsampleCommandBuffer(const uint32_t& currentFrame, const uint32_t& mipLevel) { return this->upCommandBuffers[currentFrame][mipLevel]; }
	inline ShaderInput& getDownsampleShaderInput() { return this->downShaderInput; }
	inline ShaderInput& getUpsampleShaderInput() { return this->upShaderInput; }
	inline ResolutionPushConstantData& getResolutionData() { return this->resolutionData; }
	inline const Texture& getDepthTexture() const { return this->depthTexture; }
	inline const RenderPass& getDownsampleRenderPass() const { return this->downRenderPass; }
	inline const RenderPass& getUpsampleRenderPass() const { return this->upRenderPass; }
	inline const Pipeline& getDownsamplePipeline() const { return this->downPipeline; }
	inline const Pipeline& getUpsamplePipeline() const { return this->upPipeline; }
	inline const vk::Framebuffer& getRenderVkFramebuffer() const { return this->renderFramebuffer[0]; }
	inline const vk::Framebuffer& getMipVkFramebuffer(const uint32_t& mipLevel) const { return this->mipFramebuffers[mipLevel]; }
	inline const vk::Extent2D& getMipExtent(const uint32_t& mipLevel) const { return this->mipExtents[mipLevel]; }
	inline const uint32_t& getMipDescriptorIndex(const uint32_t& mipLevel) const { return this->mipDescriptorIndices[mipLevel]; }
	inline const uint32_t& getDesiredNumMipLevels() const { return this->desiredNumMipLevels; }
	inline const uint32_t& getNumMipLevelsInUse() const { return this->numMipLevelsInUse; }
};