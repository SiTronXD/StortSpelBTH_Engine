#include "DescriptorSet.h"

DescriptorSet::DescriptorSet(Renderer& renderer)
	: renderer(renderer),
	descriptorSet(VK_NULL_HANDLE)
{
}

DescriptorSet::~DescriptorSet()
{
}

void DescriptorSet::setVkDescriptorSet(const VkDescriptorSet& descriptorSet)
{
	this->descriptorSet = descriptorSet;
}
