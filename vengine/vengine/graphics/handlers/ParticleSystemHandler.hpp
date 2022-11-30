#pragma once

#include "../vulkan/Pipeline.hpp"
#include "../vulkan/ShaderStructs.hpp"
#include "../vulkan/CommandBufferArray.hpp"

class Device;
class RenderPass;
class CommandBufferArray;
class Scene;

class ParticleSystemHandler
{
private:
	GlobalParticleBufferData globalParticleData;

	std::vector<ParticleInfo> initialParticleInfos;
	std::vector<ParticleEmitterInfo> particleEmitterInfos;

	// Compute
	//StorageBufferID particleInfoWriteSBO;
	//ShaderInput computeShaderInput;
	Pipeline computePipeline;
	CommandBufferArray computeCommandBuffers;

	// Graphics
	UniformBufferID cameraUBO;
	StorageBufferID particleInfoSBO;
	StorageBufferID particleEmitterInfoSBO;
	UniformBufferID globalParticleBufferUBO;
	ShaderInput shaderInput;
	Pipeline pipeline;

	PhysicalDevice* physicalDevice;
	Device* device;
	VmaAllocator* vma;
	vk::Queue* transferQueue;
	vk::CommandPool* transferCommandPool;
	ResourceManager* resourceManager;
	RenderPass* renderPass;
	vk::CommandPool* computeCommandPool;
	uint32_t framesInFlight;

	uint32_t numParticles;

public:
	ParticleSystemHandler();

	void init(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		vk::Queue& transferQueue,
		vk::CommandPool& transferCommandPool,
		ResourceManager& resourceManager,
		RenderPass& renderPass,
		vk::CommandPool& computeCommandPool,
		const uint32_t& framesInFlight);
	void initForScene(Scene* scene);
	void update(
		Scene* scene,
		const CameraBufferData& cameraDataUBO,
		const uint32_t& currentFrame);
	void cleanup();

	inline ShaderInput& getShaderInput() { return this->shaderInput; }
	inline const Pipeline& getPipeline() const { return this->pipeline; }
	inline const Pipeline& getComputePipeline() const { return this->computePipeline; }
	inline CommandBuffer& getComputeCommandBuffer(const uint32_t& index) { return this->computeCommandBuffers[index]; }
	inline const uint32_t& getNumParticles() const { return this->numParticles; }
};