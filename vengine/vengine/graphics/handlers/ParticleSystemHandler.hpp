#pragma once

#include "../vulkan/Pipeline.hpp"
#include "../vulkan/ShaderStructs.hpp"

class Device;
class RenderPass;

class ParticleSystemHandler
{
private:
	struct ParticleInfo
	{
		glm::mat4 transformMatrix = glm::mat4(1.0f);
	};

	std::vector<ParticleInfo> particleInfos;

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
};