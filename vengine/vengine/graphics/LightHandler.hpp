#pragma once

#include <vulkan/vulkan.hpp>
#include "vulkan/ShaderStructs.hpp"
#include "vulkan/RenderPass.hpp"
#include "vulkan/FramebufferArray.hpp"
#include "vulkan/CommandBufferArray.hpp"
#include "vulkan/Pipeline.hpp"
#include "ShaderInput.hpp"
#include "vulkan/CommandBufferArray.hpp"

class Scene;
class RenderPass;
class FramebufferArray;
class CommandBufferArray;
class Pipeline;
class ResourceManager;

class LightHandler
{
private:
	std::vector<LightBufferData> lightBuffer;

	ShadowMapData shadowMapData{};

	Texture shadowMapTexture;
	RenderPass shadowMapRenderPass;
	FramebufferArray shadowMapFramebuffer;
	CommandBufferArray shadowMapCommandBuffers;
	vk::Extent2D shadowMapExtent;

	// Default meshes
	UniformBufferID shadowMapViewProjectionUB;
	ShaderInput shadowMapShaderInput;
	Pipeline shadowMapPipeline;

	// Skeletal animations
	UniformBufferID animShadowMapViewProjectionUB;
	ShaderInput animShadowMapShaderInput;
	Pipeline animShadowMapPipeline;

	PhysicalDevice* physicalDevice;
	Device* device;
	VmaAllocator* vma;
	ResourceManager* resourceManager;

	uint32_t framesInFlight;

public:
	static const uint32_t MAX_NUM_LIGHTS = 16;
	static const uint32_t SHADOW_MAP_SIZE = 1024;

	LightHandler();

	void init(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		vk::CommandPool& commandPool,
		ResourceManager& resourceManager,
		const uint32_t& framesInFlight);

	void initForScene(
		Scene* scene,
		const bool& oldHasAnimations,
		const bool& hasAnimations);

	void updateLightBuffers(
		Scene* scene,
		ShaderInput& shaderInput,
		ShaderInput& animShaderInput,
		const UniformBufferID& allLightsInfoUB,
		const UniformBufferID& animAllLightsInfoUB,
		const StorageBufferID& lightBufferSB,
		const StorageBufferID& animLightBufferSB,
		const bool& hasAnimations,
		const glm::vec3& camPosition,
		const uint32_t& currentFrame);

	void cleanup();

	inline const ShadowMapData& getShadowMapData() const { return this->shadowMapData; }
	inline const RenderPass& getShadowMapRenderPass() const { return this->shadowMapRenderPass; }
	inline const vk::Extent2D& getShadowMapExtent() const { return this->shadowMapExtent; }
	inline const vk::Framebuffer& getShadowMapFramebuffer() const { return this->shadowMapFramebuffer[0]; }
	inline const Pipeline& getShadowMapPipeline() const { return this->shadowMapPipeline; }
	inline const Pipeline& getAnimShadowMapPipeline() const { return this->animShadowMapPipeline; }
	inline ShaderInput& getShadowMapShaderInput() { return this->shadowMapShaderInput; }
	inline ShaderInput& getAnimShadowMapShaderInput() { return this->animShadowMapShaderInput; }
	inline Texture& getShadowMapTexture() { return shadowMapTexture; }
	inline CommandBuffer& getShadowMapCommandBuffer(const uint32_t& index) { return this->shadowMapCommandBuffers[index]; }
};