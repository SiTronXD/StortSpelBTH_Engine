#pragma once

#include <vector>

#include "DescriptorSet.h"

class DescriptorSetLayout;
class DescriptorPool;
class Texture;

class DescriptorSetArray
{
private:
	std::vector<DescriptorSet*> descriptorSets;

	Renderer& renderer;

public:
	DescriptorSetArray(Renderer& renderer);
	~DescriptorSetArray();

	void createDescriptorSets(
		DescriptorSetLayout& descriptorSetLayout,
		DescriptorPool& descriptorPool,
		uint32_t numDescriptorSets,
		
		std::vector<VkBuffer> uniformBuffers,
		Texture& texture);

	void cleanup();

	inline DescriptorSet& getDescriptorSet(uint32_t index) { return *this->descriptorSets[index]; }
};