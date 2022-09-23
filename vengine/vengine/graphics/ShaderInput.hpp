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

	uint32_t currentFrame;
	uint32_t framesInFlight;

	UniformBuffer* viewProjectionUB;

	vk::DescriptorPool descriptorPool{};
	vk::DescriptorPool samplerDescriptorPool{};

	vk::DescriptorSetLayout descriptorSetLayout{};
	vk::DescriptorSetLayout samplerDescriptorSetLayout{};
	vk::PushConstantRange pushConstantRange{};

	std::vector<vk::DescriptorSet> descriptorSets;
	std::vector<vk::DescriptorSet> samplerDescriptorSets;

	std::vector<uint32_t> samplersTextureIndex;

	std::vector<vk::DescriptorSet> bindDescriptorSets;

	uint32_t pushConstantSize;
	vk::ShaderStageFlagBits pushConstantShaderStage;

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
	void addPushConstant(
		const uint32_t& pushConstantSize,
		const vk::ShaderStageFlagBits& pushConstantShaderStage);
	void addUniformBuffer(UniformBuffer& uniformBuffer);
	void addSampler();
	void endForInput();

	void cleanup();

	void setCurrentFrame(const uint32_t& currentFrame);
	void setTexture(
		const uint32_t& samplerIndex,
		const uint32_t& textureIndex);

	// TODO: Take in texture ID
	int addPossibleTexture(
		vk::ImageView textureImage,
		vk::Sampler& textureSampler);

	inline const vk::ShaderStageFlagBits& getPushConstantShaderStage() const { return this->pushConstantShaderStage; }
	inline const uint32_t& getPushConstantSize() const { return this->pushConstantSize; }
	inline const std::vector<vk::DescriptorSet>& getBindDescriptorSets() const 
		{ return this->bindDescriptorSets; }
	inline const PipelineLayout& getPipelineLayout() const 
		{ return this->pipelineLayout; }

	// TODO: remove these...
	
	inline vk::DescriptorSet& getDescriptorSet(const uint32_t& index) 
	{ return this->descriptorSets[index]; }

	inline vk::DescriptorSet& getSamplerDescriptorSet(const uint32_t& index)
	{ return this->samplerDescriptorSets[index]; }
};