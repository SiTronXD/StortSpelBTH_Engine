#include "DescriptorSetLayout.h"

#include "../Renderer.h"

DescriptorSetLayout::DescriptorSetLayout(Renderer& renderer)
	: renderer(renderer),
	descriptorSetLayout(VK_NULL_HANDLE)
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

void DescriptorSetLayout::createDescriptorSetLayout()
{
	// Uniform buffer object layout binding
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	// Combined image sampler layout binding
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	// Descriptor set layout info, which contains all layout bindings
	std::array<VkDescriptorSetLayoutBinding, 2> bindings =
	{
		uboLayoutBinding,
		samplerLayoutBinding
	};
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	// Create descriptor set layout
	if (vkCreateDescriptorSetLayout(
		this->renderer.getVkDevice(),
		&layoutInfo,
		nullptr,
		&this->descriptorSetLayout))
	{
		Log::error("Failed to create descriptor set layout.");
	}
}

void DescriptorSetLayout::cleanup()
{
	vkDestroyDescriptorSetLayout(this->renderer.getVkDevice(), this->descriptorSetLayout, nullptr);
}
