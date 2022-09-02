#pragma once

#include "DescriptorSetLayout.h"

class PipelineLayout
{
private:
	VkPipelineLayout pipelineLayout;

	Renderer& renderer;

public:
	PipelineLayout(Renderer& renderer);
	~PipelineLayout();

	void createPipelineLayout(const DescriptorSetLayout& descriptorSetLayout);

	void cleanup();

	inline const VkPipelineLayout& getVkPipelineLayout() const { return this->pipelineLayout; }
};