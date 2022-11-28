#pragma once

#include "../vulkan/Pipeline.hpp"
#include "../vulkan/ShaderStructs.hpp"
#include "../vulkan/CommandBufferArray.hpp"

class Device;
class RenderPass;
class CommandBufferArray;

class ParticleSystemHandler
{
private:
	struct ParticleInfo
	{
		glm::mat4 transformMatrix = glm::mat4(1.0f);
		glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	};

	std::vector<ParticleInfo> particleInfos;

	// Compute
	StorageBufferID particleInfoWriteSBO;
	//ShaderInput computeShaderInput;
	Pipeline computePipeline;
	CommandBufferArray computeCommandBuffers;

	// Graphics
	UniformBufferID cameraUBO;
	StorageBufferID particleInfoSBO;
	ShaderInput shaderInput;
	Pipeline pipeline;

public:
	inline static const uint32_t MAX_NUM_PARTICLES_PER_SYSTEM = 1024;

	void init(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		ResourceManager& resourceManager,
		RenderPass& renderPass,
		vk::CommandPool& computeCommandPool,
		const uint32_t& framesInFlight);
	void initForScene(
		PhysicalDevice& physicalDevice,
		Device& device,
		VmaAllocator& vma,
		ResourceManager& resourceManager,
		RenderPass& renderPass,
		const uint32_t& framesInFlight);
	void update(
		const CameraBufferData& cameraDataUBO,
		const uint32_t& currentFrame);
	void cleanup();

	inline ShaderInput& getShaderInput() { return this->shaderInput; }
	inline const Pipeline& getPipeline() const { return this->pipeline; }
	inline CommandBuffer& getComputeCommandBuffer(const uint32_t& index) { return this->computeCommandBuffers[index]; }
};