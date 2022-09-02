#include "DescriptorSetArray.h"
#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"

#include "../Texture.h"
#include "../Renderer.h"
#include "../../Dev/Log.h"

DescriptorSetArray::DescriptorSetArray(Renderer& renderer)
	: renderer(renderer)
{
}

DescriptorSetArray::~DescriptorSetArray()
{
	this->cleanup();
}

void DescriptorSetArray::createDescriptorSets(
	DescriptorSetLayout& descriptorSetLayout,
	DescriptorPool& descriptorPool, 
	uint32_t numDescriptorSets,

	std::vector<VkBuffer> uniformBuffers,
	Texture& texture)
{
	this->descriptorSets.resize(numDescriptorSets);
	std::vector<VkDescriptorSet> descriptorSetData(numDescriptorSets);

	// Allocate descriptor sets
	std::vector<VkDescriptorSetLayout> layouts(
		numDescriptorSets,
		descriptorSetLayout.getVkDescriptorSetLayout()
	);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool.getVkDescriptorPool();
	allocInfo.descriptorSetCount = numDescriptorSets;
	allocInfo.pSetLayouts = layouts.data();
	if (vkAllocateDescriptorSets(
		this->renderer.getVkDevice(), 
		&allocInfo, 
		descriptorSetData.data()) != VK_SUCCESS)
	{
		Log::error("Failed to allocate descriptor sets.");
	}

	// Populate descriptor sets
	for (size_t i = 0; i < descriptorSetData.size(); ++i)
	{
		this->descriptorSets[i] = new DescriptorSet(this->renderer);
		this->descriptorSets[i]->setVkDescriptorSet(descriptorSetData[i]);
	}

	// Populate descriptor sets
	for (size_t i = 0; i < numDescriptorSets; ++i)
	{
		// Descriptor buffer info
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		// Descriptor image info
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture.getVkImageView();
		imageInfo.sampler = texture.getVkSampler();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		// Write descriptor set for uniform buffer
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = this->descriptorSets[i]->getVkDescriptorSet();
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		// Write descriptor set for image sampler
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = this->descriptorSets[i]->getVkDescriptorSet();
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(
			this->renderer.getVkDevice(),
			static_cast<uint32_t>(descriptorWrites.size()),
			descriptorWrites.data(),
			0,
			nullptr
		);
	}
}

void DescriptorSetArray::cleanup()
{
	for (size_t i = 0; i < this->descriptorSets.size(); ++i)
	{
		delete this->descriptorSets[i];
		this->descriptorSets[i] = nullptr;
	}

	this->descriptorSets.clear();
}
