#pragma once

#include "vulkan/PipelineLayout.hpp"
#include "vulkan/UniformBuffer.hpp"

class PhysicalDevice;
class Device;

using UniformBufferID = uint32_t;
using SamplerID = uint32_t;

enum class DescriptorFrequency
{
	PER_FRAME,
	// PER_MESH, // TODO: add functionality for this
	PER_DRAW_CALL
};

class ShaderInput
{
private:
	const uint32_t MAX_NUM_TEXTURES = 250;

	PipelineLayout pipelineLayout;

	PhysicalDevice* physicalDevice;
	Device* device;
	VmaAllocator* vma;

	uint32_t currentFrame;
	uint32_t framesInFlight;

	vk::DescriptorPool descriptorPool{};
	vk::DescriptorPool samplerDescriptorPool{};

	vk::DescriptorSetLayout descriptorSetLayout{};
	vk::DescriptorSetLayout samplerDescriptorSetLayout{};
	vk::PushConstantRange pushConstantRange{};

	std::vector<UniformBuffer> addedUniformBuffers;

	std::vector<vk::DescriptorSet> perFrameDescriptorSets;
	// std::vector<vk::DescriptorSet> perMeshDescriptorSets; // TODO: add functionality for this
	std::vector<vk::DescriptorSet> perDrawDescriptorSets;

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
		PhysicalDevice& physicalDevice,
		Device& device, 
		VmaAllocator& vma,
		const uint32_t& framesInFlight);
	void addPushConstant(
		const uint32_t& pushConstantSize,
		const vk::ShaderStageFlagBits& pushConstantShaderStage);
	UniformBufferID addUniformBuffer(
		const size_t& contentsSize);
	SamplerID addSampler();
	void endForInput();

	void cleanup();

	void updateUniformBuffer(
		const UniformBufferID& id,
		void* data,
		const uint32_t& currentFrame);
	void setCurrentFrame(const uint32_t& currentFrame);
	void setTexture(
		const SamplerID& samplerID,
		const uint32_t& textureIndex);

	// TODO: Take in texture ID
	int addPossibleTexture(
		vk::ImageView textureImageView,
		vk::Sampler& textureSampler);

	inline const vk::ShaderStageFlagBits& getPushConstantShaderStage() const { return this->pushConstantShaderStage; }
	inline const uint32_t& getPushConstantSize() const { return this->pushConstantSize; }
	inline const std::vector<vk::DescriptorSet>& getBindDescriptorSets() const 
		{ return this->bindDescriptorSets; }
	inline const PipelineLayout& getPipelineLayout() const 
		{ return this->pipelineLayout; }
};