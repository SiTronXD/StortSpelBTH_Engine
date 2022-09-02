#pragma once

#include <vulkan/vulkan.h>

class Renderer;

class DescriptorSet
{
private:
	VkDescriptorSet descriptorSet;

	Renderer& renderer;

public:
	DescriptorSet(Renderer& renderer);
	~DescriptorSet();

	void setVkDescriptorSet(const VkDescriptorSet& descriptorSet);

	inline const VkDescriptorSet& getVkDescriptorSet() const { return this->descriptorSet; }
};

