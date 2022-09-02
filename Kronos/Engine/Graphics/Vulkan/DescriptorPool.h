#pragma once

#include <vulkan/vulkan.h>

class Renderer;

class DescriptorPool
{
private:
	VkDescriptorPool descriptorPool;

	Renderer& renderer;

public:
	DescriptorPool(Renderer& renderer);
	~DescriptorPool();

	void createDescriptorPool(uint32_t descriptorCount);
	void createImguiDescriptorPool(uint32_t descriptorCount);

	void cleanup();

	inline const VkDescriptorPool& getVkDescriptorPool() const { return this->descriptorPool; }
};