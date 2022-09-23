#pragma once

#include "vulkan/PipelineLayout.hpp"

class Device;

class ShaderInput
{
private:
	PipelineLayout pipelineLayout;

	Device* device;
	VmaAllocator* vma;

public:
	ShaderInput();
	~ShaderInput();

	void beginForInput(
		Device& device, 
		VmaAllocator& vma);
	void addUniformBuffer();
	void addPushConstant();
	void addSampler();
	void endForInput();

	void cleanup();
};