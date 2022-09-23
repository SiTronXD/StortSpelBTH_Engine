#pragma once

#include "../ShaderInput.hpp"

class Device;

class Pipeline
{
private:
	vk::Pipeline pipeline{};

	Device* device;

	vk::ShaderModule createShaderModule(
		const std::vector<char>& code);

public:
	Pipeline();
	~Pipeline();

	void createPipeline(
		Device& device,
		ShaderInput& shaderInput,
		vk::RenderPass& renderPass);

	void cleanup();

	inline const vk::Pipeline& getVkPipeline() const
	{ return this->pipeline; }
};