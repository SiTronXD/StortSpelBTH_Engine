#pragma once

#include "vulkan/PipelineLayout.hpp"
#include "vulkan/UniformBuffer.hpp"

class Device;

class ShaderInput
{
private:
	PipelineLayout pipelineLayout;

	Device* device;
	VmaAllocator* vma;

	uint32_t framesInFlight;

	UniformBuffer* viewProjectionUB;

	vk::DescriptorPool descriptorPool{};
	vk::DescriptorPool samplerDescriptorPool{};

	vk::DescriptorSetLayout descriptorSetLayout{};
	vk::DescriptorSetLayout samplerDescriptorSetLayout{};
	vk::PushConstantRange pushConstantRange{};

	std::vector<vk::DescriptorSet> descriptorSets;
	std::vector<vk::DescriptorSet> samplerDescriptorSets;

	void createDescriptorSetLayout();
	void createDescriptorPool();
	void allocateDescriptorSets();
	void createDescriptorSets();

public:
	ShaderInput();
	~ShaderInput();

	void beginForInput(
		Device& device, 
		VmaAllocator& vma,
		const uint32_t& framesInFlight);
	void addUniformBuffer(UniformBuffer& uniformBuffer);
	void addPushConstant(const uint32_t& modelMatrixSize);
	void addSampler();
	void endForInput();

	void cleanup();

	inline PipelineLayout& getPipelineLayout() { return this->pipelineLayout; }

	// TODO: remove these...
	int createSamplerDescriptor(
		vk::ImageView textureImage,
		vk::Sampler& textureSampler);

	inline vk::DescriptorSet& getDescriptorSet(const uint32_t& index) 
	{ return this->descriptorSets[index]; }

	inline vk::DescriptorSet& getSamplerDescriptorSet(const uint32_t& index)
	{ return this->samplerDescriptorSets[index]; }
};