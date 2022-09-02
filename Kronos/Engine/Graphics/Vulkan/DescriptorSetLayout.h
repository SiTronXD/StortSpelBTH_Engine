#pragma once

#include <vulkan/vulkan.h>

class Renderer;

class DescriptorSetLayout
{
private:
	VkDescriptorSetLayout descriptorSetLayout;

	Renderer& renderer;

public:
	DescriptorSetLayout(Renderer& renderer);
	~DescriptorSetLayout();

	void createDescriptorSetLayout();

	void cleanup();

	inline const VkDescriptorSetLayout& getVkDescriptorSetLayout() const 
		{ return this->descriptorSetLayout; }
};