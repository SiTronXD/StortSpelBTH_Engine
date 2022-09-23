#pragma once

#include "../TempPCH.hpp"

class Device;

class PipelineLayout
{
private:
	vk::PipelineLayout pipelineLayout{};

	Device* device;

public:
	PipelineLayout();
	~PipelineLayout();

	void createPipelineLayout(
		Device& device,
		vk::DescriptorSetLayout& descriptorSetLayout,
		vk::DescriptorSetLayout& samplerDescriptorSetLayout,
		vk::PushConstantRange& pushConstantRange);

	void cleanup();

	inline vk::PipelineLayout& getVkPipelineLayout() 
	{ return this->pipelineLayout; }
};