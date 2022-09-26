#pragma once

#include "PipelineLayout.hpp"

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
		PipelineLayout& pipelineLayout,
		vk::RenderPass& renderPass);

	void cleanup();

	inline vk::Pipeline& getVkPipeline() { return this->pipeline; }
};