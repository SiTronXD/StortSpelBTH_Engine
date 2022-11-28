#pragma once

#include "../vulkan/Pipeline.hpp"
#include "../vulkan/ShaderStructs.hpp"

class Device;
class RenderPass;

class ParticleSystemHandler
{
private:
	UniformBufferID cameraUBO;
	ShaderInput shaderInput;
	Pipeline pipeline;

public:
	void init(
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

	inline const ShaderInput& getShaderInput() const { return this->shaderInput; }
	inline const Pipeline& getPipeline() const { return this->pipeline; }
};