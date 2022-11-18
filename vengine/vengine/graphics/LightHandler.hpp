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
class Texture;
class DebugRenderer;

struct Camera;

class LightHandler
{
public:
	static const uint32_t NUM_CASCADES = 4;

private:
	std::vector<LightBufferData> lightBuffer;

	ShadowMapData shadowMapData{};

	ShadowPushConstantData shadowPushConstantData{};

	Texture shadowMapTexture;
	RenderPass shadowMapRenderPass;
	FramebufferArray shadowMapFramebuffer;
	CommandBufferArray shadowMapCommandBuffers;
	vk::Extent2D shadowMapExtent;

	// Default meshes
	ShaderInput shadowMapShaderInput;
	Pipeline shadowMapPipeline;

	// Skeletal animations
	ShaderInput animShadowMapShaderInput;
	Pipeline animShadowMapPipeline;

	PhysicalDevice* physicalDevice;
	Device* device;
	VmaAllocator* vma;
	ResourceManager* resourceManager;

	uint32_t framesInFlight;

	glm::vec3 lightDir;
	glm::vec3 lightUpDir;

	std::vector<float> cascadeNearPlanes;
	std::vector<float> cascadeSizes;
	float cascadeDepthScale;

	void getWorldSpaceFrustumCorners(
		const glm::mat4& invViewProj,
		glm::vec4 outputCorners[]);

	void setLightFrustum(
		const glm::mat4& camProj,
		const glm::mat4& camView,
		glm::mat4& outputLightVP);

public:
	static const uint32_t MAX_NUM_LIGHTS = 16;
	static const uint32_t SHADOW_MAP_SIZE = 1024 * 4;

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
		const Camera& camData,
		const uint32_t& currentFrame);

	void updateCamera(
		const uint32_t& arraySliceCameraIndex);
	void updateShadowPushConstant(
		CommandBuffer& currentShadowMapCommandBuffer,
		const glm::mat4& modelMatrix);

	void cleanup();

	inline const ShadowMapData& getShadowMapData() const { return this->shadowMapData; }
	inline const RenderPass& getShadowMapRenderPass() const { return this->shadowMapRenderPass; }
	inline const vk::Extent2D& getShadowMapExtent() const { return this->shadowMapExtent; }
	inline const vk::Framebuffer& getShadowMapFramebuffer(const uint32_t& index) const { return this->shadowMapFramebuffer[index]; }
	inline const Pipeline& getShadowMapPipeline() const { return this->shadowMapPipeline; }
	inline const Pipeline& getAnimShadowMapPipeline() const { return this->animShadowMapPipeline; }
	inline ShaderInput& getShadowMapShaderInput() { return this->shadowMapShaderInput; }
	inline ShaderInput& getAnimShadowMapShaderInput() { return this->animShadowMapShaderInput; }
	inline Texture& getShadowMapTexture() { return shadowMapTexture; }
	inline CommandBuffer& getShadowMapCommandBuffer(const uint32_t& index) { return this->shadowMapCommandBuffers[index]; }
};