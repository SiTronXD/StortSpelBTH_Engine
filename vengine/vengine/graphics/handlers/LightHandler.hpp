#pragma once

#include <vulkan/vulkan.hpp>
#include "../vulkan/ShaderStructs.hpp"
#include "../vulkan/RenderPass.hpp"
#include "../vulkan/FramebufferArray.hpp"
#include "../vulkan/CommandBufferArray.hpp"
#include "../vulkan/Pipeline.hpp"
#include "../ShaderInput.hpp"
#include "../vulkan/CommandBufferArray.hpp"

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
	static const uint32_t NUM_CASCADES;

private:
	std::vector<LightBufferData> lightBuffer;

	ShadowMapData shadowMapData{};
	ShadowPushConstantData shadowPushConstantData{};
	ShadowMapCameraBufferData shadowMapCameraBufferData{};

	Texture shadowMapTexture;
	RenderPass shadowMapRenderPass;
	FramebufferArray shadowMapFramebuffer;
	CommandBufferArray shadowMapCommandBuffers;
	vk::Extent2D shadowMapExtent;

	// Default meshes
	ShaderInput shadowMapShaderInput;
	UniformBufferID shadowMapVpUbo;
	Pipeline shadowMapPipeline;

	// Skeletal animations
	ShaderInput animShadowMapShaderInput;
	UniformBufferID animShadowMapVpUbo;
	Pipeline animShadowMapPipeline;

	PhysicalDevice* physicalDevice;
	Device* device;
	VmaAllocator* vma;
	ResourceManager* resourceManager;

	uint32_t framesInFlight;
	uint32_t numLights;

	glm::mat4 lightViewMat;
	glm::vec3 lightDir;
	glm::vec3 lightUpDir;

	std::vector<float> cascadeSizes;
	float cascadeDepthScale;

	void addLightToList(
		LightBufferData& lightData);
	void setLightFrustum(
		const float& cascadeSize,
		glm::mat4& outputLightVP);

public:
	static const uint32_t MAX_NUM_LIGHTS;
	static const uint32_t SHADOW_MAP_SIZE;

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

	void updateDefaultShadowPushConstant(
		CommandBuffer& currentShadowMapCommandBuffer,
		const glm::mat4& modelMatrix);
	void updateAnimatedShadowPushConstant(
		CommandBuffer& currentShadowMapCommandBuffer,
		const glm::mat4& modelMatrix);

	void cleanup(const bool& hasAnimations);

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